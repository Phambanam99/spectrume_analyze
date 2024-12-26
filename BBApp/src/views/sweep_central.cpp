#include "sweep_central.h"

#include <QBoxLayout>
#include <QToolBar>
#include <QSplitter>
#include <QTreeView>
#include <QKeyEvent>

#include "trace_view.h"
#include "../mainwindow.h"
#include "../model/session.h"
#include "../model/trace.h"
#include "../model/playback_toolbar.h"
#include "../widgets/entry_widgets.h"
#include "../widgets/audio_dialog.h"
#include "../lib/acquisition.h"
#include <iostream>
SweepCentral::SweepCentral(Session *sPtr,
                           QToolBar *toolBar,
                           QWidget *parent,
                           Qt::WindowFlags f)
    : CentralWidget(parent, f),
      session_ptr(sPtr),
      trace(true),
      programClosing(false)
{
    trace_view = new TraceView(session_ptr, this);
    connect(this, SIGNAL(updateView()), trace_view, SLOT(update()));

    tools.push_back(toolBar->addWidget(new FixedSpacer(QSize(10, TOOLBAR_H))));
    Label *waterfall_label = new Label(tr("Spectrogram"), toolBar);
    waterfall_label->resize(100, 25);
    tools.push_back(toolBar->addWidget(waterfall_label));
    tools.push_back(toolBar->addWidget(new FixedSpacer(QSize(10, TOOLBAR_H))));

    waterfall_combo = new ComboBox(toolBar);
    waterfall_combo->setFixedSize(150, 25);
    QStringList spectrogram_list;
    spectrogram_list << tr("Off") << tr("2-D") << tr("3-D");
    waterfall_combo->insertItems(0, spectrogram_list);
    tools.push_back(toolBar->addWidget(waterfall_combo));
    connect(waterfall_combo, SIGNAL(currentIndexChanged(int)),
            trace_view, SLOT(enableWaterfall(int)));

    tools.push_back(toolBar->addWidget(new FixedSpacer(QSize(10, TOOLBAR_H))));
    tools.push_back(toolBar->addSeparator());
    tools.push_back(toolBar->addWidget(new FixedSpacer(QSize(10, TOOLBAR_H))));

    persistence_check = new QCheckBox(tr("Persistence"));
    persistence_check->setObjectName("SH_CheckBox");
    persistence_check->setFixedSize(120, 25);
    sweepOnlyActions.push_back(toolBar->addWidget(persistence_check));
    connect(persistence_check, SIGNAL(stateChanged(int)),
            trace_view, SLOT(enablePersistence(int)));

    persistence_clear = new SHPushButton(tr("Clear"), toolBar);
    persistence_clear->setFixedSize(100, TOOLBAR_H - 4);
    sweepOnlyActions.push_back(toolBar->addWidget(persistence_clear));
    connect(persistence_clear, SIGNAL(clicked()), trace_view, SLOT(clearPersistence()));

    if(!trace_view->HasOpenGL3()) {
        persistence_check->setEnabled(false);
        persistence_check->setToolTip(tr("Persistence requires OpenGL version 3.0 or greater"));
        persistence_clear->setEnabled(false);
        persistence_clear->setToolTip(tr("Persistence requires OpenGL version 3.0 or greater"));
    }

    realTimePersistenceCheck = new QCheckBox("Persistence");
    realTimePersistenceCheck->setObjectName("SH_CheckBox");
    realTimePersistenceCheck->setFixedSize(120, 25);
    realTimePersistenceCheck->setChecked(true);
    realTimeActions.push_back(toolBar->addWidget(realTimePersistenceCheck));
    connect(realTimePersistenceCheck, SIGNAL(stateChanged(int)),
            trace_view, SLOT(enableRealTimePersist(int)));

    Label *intensityLabel = new Label(tr("Intensity  "), toolBar);
    intensityLabel->resize(100, 25);
    realTimeActions.push_back(toolBar->addWidget(intensityLabel));
    intensitySlider = new QSlider(Qt::Horizontal, this);
    intensitySlider->setFixedSize(200, 25);
    intensitySlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    intensitySlider->setTracking(true);
    intensitySlider->setEnabled(true);
    intensitySlider->setRange(1, 99);
    intensitySlider->setSliderPosition(25);
    realTimeActions.push_back(toolBar->addWidget(intensitySlider));
    connect(intensitySlider, SIGNAL(valueChanged(int)),
            trace_view, SLOT(intensityChanged(int)));

    if(!trace_view->CanDrawRealTimePersistence()) {
        realTimePersistenceCheck->setEnabled(false);
        realTimePersistenceCheck->setToolTip("Real-time persistence is not supported by "
                                             "your graphics card.");
        intensitySlider->setEnabled(false);
        intensitySlider->setToolTip("Real-time persistence is not supported by your "
                                    "graphics card.");
    }

    for(QAction *a : realTimeActions) {
        a->setVisible(false);
    }

    trace_view->move(0, 0);
    trace_view->resize(size().width(), size().height() - 32);

    playback = new PlaybackToolBar(&session_ptr->prefs, this);
    playback->move(0, trace_view->height());
    playback->resize(size().width(), 32);

    sweeping = true;
    sweep_count = -1;
    reconfigure = false;

    connect(session_ptr->sweep_settings, SIGNAL(updated(const SweepSettings*)),
            this, SLOT(settingsChanged(const SweepSettings*)));
    connect(session_ptr->trace_manager, SIGNAL(updated()),
            this, SLOT(forceUpdateView()));
    connect(playback, SIGNAL(startPlaying(bool)),
            this, SLOT(playFromFile(bool)));

    //StartStreaming();
    setFocusPolicy(Qt::StrongFocus);
}

SweepCentral::~SweepCentral()
{
    programClosing = true;
    playback->Stop();
    StopStreaming();

    //delete tool_bar;
    delete trace_view;
    delete playback;
}

void SweepCentral::changeMode(int new_state)
{
    StopStreaming();
    sweep_count = -1;

    session_ptr->sweep_settings->setMode((OperationalMode)new_state);

    if(new_state == MODE_REAL_TIME) {
        for(QAction *a : sweepOnlyActions) { a->setVisible(false); }
        for(QAction *a : realTimeActions) { a->setVisible(true); }
    } else {
        for(QAction *a : sweepOnlyActions) { a->setVisible(true); }
        for(QAction *a : realTimeActions) { a->setVisible(false); }
    }

    if(new_state == BB_SWEEPING || new_state == BB_REAL_TIME) {
        qDebug() <<"start streaming";
        StartStreaming();
    }
}

void SweepCentral::StartStreaming()
{
    sweeping = true;

    thread_handle = std::thread(&SweepCentral::SweepThread, this);
}

void SweepCentral::StopStreaming()
{
    if(thread_handle.joinable()) {
        sweeping = false;
        thread_handle.join();
    }
}

void SweepCentral::ResetView()
{
    persistence_check->setChecked(false);
    waterfall_combo->setCurrentIndex(WaterfallOFF);
}

void SweepCentral::GetViewImage(QImage &image)
{
    image = trace_view->grabFrameBuffer();
}

void SweepCentral::resizeEvent(QResizeEvent *)
{
    trace_view->resize(width(), height() - 32);
    playback->move(0, trace_view->height());
    playback->resize(width(), 32);
}

void SweepCentral::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Left || e->key() == Qt::Key_Right) {
        session_ptr->trace_manager->BumpMarker(e->key() == Qt::Key_Right);
        trace_view->update();
    }
}

void SweepCentral::wheelEvent(QWheelEvent *we)
{
    session_ptr->trace_manager->BumpMarker(we->delta() > 0);
    trace_view->update();
}

// Try new settings
// If new settings fail, revert to old settings
void SweepCentral::Reconfigure()
{   if(device_traits::get_device_rtl())
    {

        bool check = session_ptr ->device->Reconfigure(session_ptr->sweep_settings, &trace);
    if(check){
         last_config = *session_ptr->sweep_settings;
    } else
    if(!session_ptr->device->Reconfigure(session_ptr->sweep_settings, &trace)) {

        *session_ptr->sweep_settings = last_config;
    }

    if(last_config.Mode() == MODE_REAL_TIME) {
        rtFrame.SetDimensions(session_ptr->device->RealTimeFrameSize());
    }

    if(sweep_count == 0) {
        sweep_count = 1;
    }
    reconfigure = false;
}
}
void SweepCentral::ReconfiugreRtl(DeviceRtlSdr *rtldev ){
         last_config = *session_ptr->sweep_settings;
//         last_config.Span().Val() = rtldev -> sample_rate() ;
         rtldev->Reconfigure(&last_config, &trace);
         params.N = 1024;                      // Số bins trong FFT
         params.window = true;                 // Sử dụng window function
         params.baseline = true;               // Sử dụng baseline correction
         params.cropPercentage = 10.0;         // Loại bỏ 10% bins ở hai biên FFT
         params.hops = 1;
         params.sample_rate = (int)rtldev -> sample_rate();
         params.startfreq = last_config.Center().Val() - (int)params.sample_rate/2; // Chỉ quét một lần
         params.stopfreq = last_config.Center().Val() - (int)params.sample_rate/2;
         params.dev_index = 0;                 // Chỉ số thiết bị RTL-SDR
         params.gain = rtldev -> gains()[last_config.Gain()];                    // Độ khuếch đại (gain)
                 // Tốc độ mẫu (sample rate)
         params.cfreq = (int) last_config.Center().Val();
           qDebug() << "add sweep" <<    &last_config ;

         qDebug() << "rtl fre " <<     params.cfreq  ;

    if(sweep_count == 0) {
        sweep_count = 1;
    }
    reconfigure = false;
}

bool SweepCentral::runReceiver_(Trace *trace, Params params, Plan *plan,
                                DeviceRtlSdr *rtldev, AuxData *auxData, Datastore *data )
{

    //Read from device and do FFT
        for (auto iter = plan -> freqs_to_tune.begin(); iter != plan ->freqs_to_tune.end();) {
            // Begin a new data acquisition.
            Acquisition *acquisition = new Acquisition(params, *auxData, *rtldev, *data, params.sample_rate, *iter);
            try {
                // Read the required amount of data and process it.
                acquisition ->run();
                iter++;
            }
            catch (TuneError &e) {
                // The receiver was unable to tune to this frequency. It might be just a "dead"
                // spot of the receiver. Remove this frequency from the list and continue.
//                std::cerr << "Unable to tune to " << e.frequency() << ". Dropping from frequency list." << std::endl;
                iter = plan -> freqs_to_tune.erase(iter);
                continue;
            }
            // Print a summary (number of samples, readouts etc.) to stderr.
//             if( (params.outcnt == 0 && params.talkless) || (params.talkless==false) ) acquisition -> print_summary();
            // Write the gathered data to stdout.
            float *  pwr = acquisition -> caculatePwr();
            trace -> setMin(pwr);
            trace -> setMax(pwr);

        }
        return true;
      
}
// Main sweep loop
void SweepCentral::SweepThread()
{
     if(session_ptr->device->GetDeviceType() == DeviceTypeRtlSdr){
         qDebug() << 1;
     SweepThreadRtl();
     } else {
         qDebug() << 2;
         SweepThreadSignalHound();
     }
}
void SweepCentral::SweepThreadRtl()
{
     DeviceRtlSdr *rtldev;
     AuxData *auxData;
     int actual_samplerate;
     Plan *plan ;
     Datastore *data;

     rtldev = static_cast<DeviceRtlSdr*>(session_ptr->device);
     ReconfiugreRtl(rtldev);
     auxData = new AuxData(params);
     actual_samplerate = rtldev -> sample_rate();
     plan = new Plan(params, actual_samplerate);
     plan->print();
     data = new Datastore(params, auxData ->window_values);
     params.finalfreq = plan -> freqs_to_tune.back();

    while(sweeping) {
        if(reconfigure) {
            qDebug() << "wtf" ;
          ReconfiugreRtl(rtldev);
           session_ptr->trace_manager->ClearAllTraces();
        }

        if(sweep_count) {
            bool sweepSuccess;
            if(last_config.Mode() == MODE_REAL_TIME) {
                sweepSuccess = session_ptr->device->GetRealTimeFrame(trace, rtFrame);
            } else {
//
                sweepSuccess =  runReceiver_(&trace, params,  plan,
                                rtldev,  auxData,  data);
            }

            if(!sweepSuccess) {
                sweeping = false;
                return;
            }

            if(trace.IsFullSweep()) {
                playback->PutTrace(&trace);
            }

            session_ptr->trace_manager->UpdateTraces(&trace);

            if(last_config.IsRealTime()) {
                session_ptr->trace_manager->realTimeFrame = rtFrame;
            }
            emit updateView();

            // Non-negative sweep count means we only collect 'n' more sweeps
            if(sweep_count > 0 && trace.IsFullSweep()) {
                sweep_count--;
            }

            // Artificial sweep delay, should be used sparingly
            if((session_ptr->sweep_settings->Mode() == BB_SWEEPING) &&
                    (session_ptr->prefs.sweepDelay > 0)) {
                Sleep(session_ptr->prefs.sweepDelay);
            }

        } else {
            Sleep(10);
        }
    }

    session_ptr->device->Abort();
}
void SweepCentral::SweepThreadSignalHound()
{
    Reconfigure();
    while(sweeping) {
        if(reconfigure) {
            Reconfigure();
            session_ptr->trace_manager->ClearAllTraces();
        }

        if(sweep_count) {
            bool sweepSuccess;
            if(last_config.Mode() == MODE_REAL_TIME) {
                sweepSuccess = session_ptr->device->GetRealTimeFrame(trace, rtFrame);
            } else {
                if(session_ptr->device->GetDeviceType() != DeviceTypeRtlSdr){
                 sweepSuccess = session_ptr->device->GetSweep(&last_config, &trace);}
            }

            if(!sweepSuccess) {
                sweeping = false;
                return;
            }

            if(trace.IsFullSweep()) {
                playback->PutTrace(&trace);
            }

            session_ptr->trace_manager->UpdateTraces(&trace);

            if(last_config.IsRealTime()) {
                session_ptr->trace_manager->realTimeFrame = rtFrame;
            }
            emit updateView();

            // Non-negative sweep count means we only collect 'n' more sweeps
            if(sweep_count > 0 && trace.IsFullSweep()) {
                sweep_count--;
            }

            // Artificial sweep delay, should be used sparingly
            if((session_ptr->sweep_settings->Mode() == BB_SWEEPING) &&
                    (session_ptr->prefs.sweepDelay > 0)) {
                Sleep(session_ptr->prefs.sweepDelay);
            }

        } else {
            Sleep(10);
        }
    }

    session_ptr->device->Abort();
}

/*
 * Save current settings and title to temporaries
 * When finally complete, restore them
 */
void SweepCentral::PlaybackThread()
{
    SweepSettings playback_settings, temp_settings;
    QString playback_title, temp_title;

    temp_settings = *session_ptr->sweep_settings;
    temp_title = session_ptr->GetTitle();

    playback->GetPlaybackSettings(&playback_settings, playback_title);

    trace.SetSettings(playback_settings);
    *session_ptr->sweep_settings = playback_settings;
    session_ptr->SetTitle(playback_title);

    // During playback, updates to sweeps settings do nothing
    disconnect(session_ptr->sweep_settings, SIGNAL(updated(const SweepSettings*)),
               this, SLOT(settingsChanged(const SweepSettings*)));

    while(playback->GetTrace(&trace) && sweeping) {   
        session_ptr->trace_manager->UpdateTraces(&trace);
        trace_view->update();
    }

    // Restore sweep settings before the playback stop?
    session_ptr->SetTitle(temp_title);
    *session_ptr->sweep_settings = temp_settings;

    // Force stop the playback
    playback->Stop();

    // Reattach settings updates
    connect(session_ptr->sweep_settings, SIGNAL(updated(const SweepSettings*)),
            this, SLOT(settingsChanged(const SweepSettings*)));
}

void SweepCentral::settingsChanged(const SweepSettings *ss)
{
    // Do nothing if
    //if(session_ptr->sweep_settings->Mode() != BB_SWEEPING &&
    //        session_ptr->sweep_settings->mode != BB_REAL_TIME) return;

    // If reconfigure already queued, wait for it
    //while(reconfigure && session_ptr->device->IsOpen()) { Sleep(1); }
    // Queue another
    reconfigure = true;
}

void SweepCentral::singleSweepPressed()
{
    if(sweep_count <= 0) {
        sweep_count = 1;
    }
}

void SweepCentral::continuousSweepPressed()
{
    sweep_count = -1;
}

// Just update view
void SweepCentral::forceUpdateView()
{
    emit updateView();
}

void SweepCentral::playFromFile(bool play)
{
    StopStreaming();

    if(play) {
        sweeping = true;
        session_ptr->isInPlaybackMode = true;
        thread_handle = std::thread(&SweepCentral::PlaybackThread, this);
    } else {
        session_ptr->isInPlaybackMode = false;
        emit updateView();
        if(session_ptr->device->IsOpen() && !programClosing) {
            StartStreaming();
        }
    }
}
