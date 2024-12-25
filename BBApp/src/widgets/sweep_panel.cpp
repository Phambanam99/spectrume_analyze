#include "sweep_panel.h"

#include "model/session.h"
//#include "model/sweep_settings.h"
//#include "model/device.h"
SweepPanel::SweepPanel(const QString &title,
                       QWidget *parent) : DockPanel(title, parent){

}
SweepPanel::SweepPanel(const QString &title,
                       QWidget *parent,
                       Session *session)
    : DockPanel(title, parent),
      sessionPtr(session)
{   
    const SweepSettings *settings  = session -> sweep_settings;
    const DeviceType deviceType = session -> device->GetDeviceType();
    tg_page = new DockPage("Tracking Generator");
    frequency_page = new DockPage(tr("Frequency"));
    amplitude_page = new DockPage(tr("Amplitude"));
    bandwidth_page = new DockPage(tr("Bandwidth"));
    acquisition_page = new DockPage(tr("Acquisition"));
    tgSweepSize = new NumericEntry("Sweep Size", 100, "pts");
    tgHighDynamicRange = new CheckBoxEntry("High Range");
    tgSweepType = new ComboEntry("Sweep Type");
    QStringList sweepType_sl;
    sweepType_sl << "Passive Device" << "Active Device";
    tgSweepType->setComboText(sweepType_sl);

    tgStoreThru = new DualButtonEntry("Store Thru", "Store 20dB Pad");
    connect(tgStoreThru, SIGNAL(leftPressed()), this, SLOT(storeThrough()));
    connect(tgStoreThru, SIGNAL(rightPressed()), this, SLOT(storeThroughPad()));


    center = new FreqShiftEntry(tr("Center"), 0.0);
    span = new FreqShiftEntry(tr("Span"), 0.0);
    start = new FrequencyEntry(tr("Start"), 0.0);
    stop = new FrequencyEntry(tr("Stop"), 0.0);
    step = new FrequencyEntry(tr("Step"), 0.0);
    full_zero_span = new DualButtonEntry(tr("Full Span"), tr("Zero Span"));
    gainRtl = new ComboEntry(tr("Gains"));
    if(deviceType == DeviceTypeRtlSdr){
        QStringList gainsRtl;
       gainsRtl << tr("0.0") << tr("0.9") << tr("1.4") << tr("2.7")
               << tr("3.7") << tr("7.7") << tr("8.7") << tr("12.5")
               << tr("14.4") << tr("15.7") << tr("16.6") << tr("19.7")
               << tr("20.7") << tr("22.9") << tr("25.4") << tr("28.0")
               << tr("29.7") << tr("32.8") << tr("33.8") << tr("36.4")
               << tr("37.2") << tr("38.6") << tr("40.2") << tr("42.1")
               << tr("43.4") << tr("43.9") << tr("44.5") << tr("48.0")
               << tr("49.6");
        gainRtl->setComboText(gainsRtl);
        gainRtl->setEnabled(true);
    }
    qDebug() << "hereee";
    sampleRateRtrl = new ComboEntry(tr("Sample Rate"));
    if(deviceType == DeviceTypeRtlSdr){
         QStringList sampleRatesTxt;
           sampleRatesTxt << tr("250KHz") << tr("1.024MHz") << tr("1.536MHz") << tr("1.792MHz")
                          << tr("1.92MHz")  << tr("2.048MHz") << tr("2.16MHz")
                          << tr("2.4MHz") << tr("2.56MHz") <<  tr("2.88MHz") <<  tr("3.2MHz") ;
        sampleRateRtrl->setComboText(sampleRatesTxt);
        sampleRateRtrl->setEnabled(true);
    }

    ref = new AmplitudeEntry(tr("Ref"), 0.0);
    div = new NumericEntry(tr("Div"), 1.0, tr("dB"));
    gain = new ComboEntry(tr("Gain"));
    QStringList gain_sl;
    gain_sl << tr("Auto Gain") << tr("Gain 0") << tr("Gain 1") <<
               tr("Gain 2") << tr("Gain 3");
    gain->setComboText(gain_sl);
    gain->setEnabled(false);

    atten = new ComboEntry(tr("Atten"));
    QStringList atten_sl;
    atten_sl << tr("Auto Atten") << tr("0 dB") << tr("10 dB")
             << tr("20 dB") << tr("30 dB");
    atten->setComboText(atten_sl);
    atten->setEnabled(false);

    preamp = new ComboEntry(tr("Preamp"));
    QStringList preamp_sl;
    preamp_sl << "Auto" << "Off" << "On";
    preamp->setComboText(preamp_sl);
    preamp->setEnabled(false);

    native_rbw = new CheckBoxEntry("Native RBW");
    rbw = new FreqShiftEntry(tr("RBW"), 0.0);
    vbw = new FreqShiftEntry(tr("VBW"), 0.0);
    auto_rbw = new CheckBoxEntry(tr("Auto RBW"));
    auto_vbw = new CheckBoxEntry(tr("Auto VBW"));

    video_units = new ComboEntry(tr("Video Units"));
    QStringList video_sl;
    video_sl << tr("Log") << tr("Voltage") <<
                tr("Power") << tr("Sample");
    video_units->setComboText(video_sl);

    detector = new ComboEntry(tr("Detector"));
    QStringList detector_sl;
    detector_sl << tr("Min/Max") << tr("Average");
    detector->setComboText(detector_sl);

    sweep_time = new TimeEntry(tr("Swp Time"), Time(0.0), MILLISECOND);

    tg_page->AddWidget(tgSweepSize);
    tg_page->AddWidget(tgSweepType);
    tg_page->AddWidget(tgHighDynamicRange);
    tg_page->AddWidget(tgStoreThru);


    frequency_page->AddWidget(center);
    frequency_page->AddWidget(span);

    if(deviceType == DeviceTypeRtlSdr){

      frequency_page->AddWidget(sampleRateRtrl);
      frequency_page -> AddWidget(gainRtl);
    }else {
        frequency_page->AddWidget(start);
        frequency_page->AddWidget(stop);
        frequency_page->AddWidget(step);
        frequency_page->AddWidget(full_zero_span);
}


    amplitude_page->AddWidget(ref);
    amplitude_page->AddWidget(div);
    amplitude_page->AddWidget(gain);
    amplitude_page->AddWidget(atten);
    amplitude_page->AddWidget(preamp);

    bandwidth_page->AddWidget(native_rbw);
    bandwidth_page->AddWidget(rbw);
    bandwidth_page->AddWidget(vbw);
    bandwidth_page->AddWidget(auto_rbw);
    bandwidth_page->AddWidget(auto_vbw);

    acquisition_page->AddWidget(video_units);
    acquisition_page->AddWidget(detector);
    acquisition_page->AddWidget(sweep_time);

    if(DeviceTypeRtlSdr != deviceType){
         AppendPage(tg_page);
         AppendPage(amplitude_page);
         AppendPage(bandwidth_page);
         AppendPage(acquisition_page);

    }

    AppendPage(frequency_page);


    // Set panel and connect here
    updatePanel(settings);

    connect(settings, SIGNAL(updated(const SweepSettings*)),
            this, SLOT(updatePanel(const SweepSettings*)));

    connect(tgSweepSize, SIGNAL(valueChanged(double)),
            settings, SLOT(setTgSweepSize(double)));
    connect(tgHighDynamicRange, SIGNAL(clicked(bool)),
            settings, SLOT(setTgHighRange(bool)));
    connect(tgSweepType, SIGNAL(comboIndexChanged(int)),
            settings, SLOT(setTgPassiveDevice(int)));


    connect(center, SIGNAL(freqViewChanged(Frequency)),
            settings, SLOT(setCenter(Frequency)));
    connect(sampleRateRtrl, SIGNAL(comboIndexChanged(int)),
            settings, SLOT(setSampleRate(int)));
    connect(center, SIGNAL(shift(bool)),
            settings, SLOT(increaseCenter(bool)));
    connect(span, SIGNAL(freqViewChanged(Frequency)),
            settings, SLOT(setSpan(Frequency)));
    connect(span, SIGNAL(shift(bool)),
            settings, SLOT(increaseSpan(bool)));
    connect(start, SIGNAL(freqViewChanged(Frequency)),
            settings, SLOT(setStart(Frequency)));
    connect(stop, SIGNAL(freqViewChanged(Frequency)),
            settings, SLOT(setStop(Frequency)));
    connect(step, SIGNAL(freqViewChanged(Frequency)),
            settings, SLOT(setStep(Frequency)));
    connect(full_zero_span, SIGNAL(leftPressed()),
            settings, SLOT(setFullSpan()));
    connect(full_zero_span, SIGNAL(rightPressed()),
            this, SIGNAL(zeroSpanPressed()));

    connect(ref, SIGNAL(amplitudeChanged(Amplitude)),
            settings, SLOT(setRefLevel(Amplitude)));
    connect(ref, SIGNAL(shift(bool)),
            settings, SLOT(shiftRefLevel(bool)));
    connect(div, SIGNAL(valueChanged(double)),
            settings, SLOT(setDiv(double)));
    connect(gain, SIGNAL(comboIndexChanged(int)),
            settings, SLOT(setGain(int)));
    connect(atten, SIGNAL(comboIndexChanged(int)),
            settings, SLOT(setAttenuation(int)));
    connect(preamp, SIGNAL(comboIndexChanged(int)),
            settings, SLOT(setPreAmp(int)));

    connect(native_rbw, SIGNAL(clicked(bool)),
            settings, SLOT(setNativeRBW(bool)));
    connect(rbw, SIGNAL(freqViewChanged(Frequency)),
            settings, SLOT(setRBW(Frequency)));
    connect(rbw, SIGNAL(shift(bool)),
            settings, SLOT(rbwIncrease(bool)));
    connect(vbw, SIGNAL(freqViewChanged(Frequency)),
            settings, SLOT(setVBW(Frequency)));
    connect(vbw, SIGNAL(shift(bool)),
            settings, SLOT(vbwIncrease(bool)));
    connect(auto_rbw, SIGNAL(clicked(bool)), settings, SLOT(setAutoRbw(bool)));
    connect(auto_vbw, SIGNAL(clicked(bool)), settings, SLOT(setAutoVbw(bool)));

    connect(video_units, SIGNAL(comboIndexChanged(int)),
            settings, SLOT(setProcUnits(int)));
    connect(detector, SIGNAL(comboIndexChanged(int)),
            settings, SLOT(setDetector(int)));
    connect(sweep_time, SIGNAL(timeChanged(Time)),
            settings, SLOT(setSweepTime(Time)));
}
SweepPanel::~SweepPanel()
{
    // Don't delete widgets on pages
    delete tg_page;
}
void SweepPanel::init(const QString &title, QWidget *parent, Session *session){
    
    sessionPtr = session;
    tg_page = new DockPage("Tracking Generator");
    frequency_page = new DockPage(tr("Frequency"));
    amplitude_page = new DockPage(tr("Amplitude"));
    bandwidth_page = new DockPage(tr("Bandwidth"));
    acquisition_page = new DockPage(tr("Acquisition"));

    tgSweepSize = new NumericEntry("Sweep Size", 100, "pts");
    tgHighDynamicRange = new CheckBoxEntry("High Range");

    tgSweepType = new ComboEntry("Sweep Type");
    QStringList sweepType_sl;
    sweepType_sl << "Passive Device" << "Active Device";
    tgSweepType->setComboText(sweepType_sl);

    tgStoreThru = new DualButtonEntry("Store Thru", "Store 20dB Pad");
    connect(tgStoreThru, SIGNAL(leftPressed()), this, SLOT(storeThrough()));
    connect(tgStoreThru, SIGNAL(rightPressed()), this, SLOT(storeThroughPad()));

    center = new FreqShiftEntry(tr("Center"), 0.0);
    span = new FreqShiftEntry(tr("Span"), 0.0);
    start = new FrequencyEntry(tr("Start"), 0.0);
    stop = new FrequencyEntry(tr("Stop"), 0.0);
    step = new FrequencyEntry(tr("Step"), 0.0);
    full_zero_span = new DualButtonEntry(tr("Full Span"), tr("Zero Span"));

    ref = new AmplitudeEntry(tr("Ref"), 0.0);
    div = new NumericEntry(tr("Div"), 1.0, tr("dB"));
    gain = new ComboEntry(tr("Gain"));
    QStringList gain_sl;
    gain_sl << tr("Auto Gain") << tr("Gain 0") << tr("Gain 1") <<
               tr("Gain 2") << tr("Gain 3");
    gain->setComboText(gain_sl);
    gain->setEnabled(false);

    atten = new ComboEntry(tr("Atten"));
    QStringList atten_sl;
    atten_sl << tr("Auto Atten") << tr("0 dB") << tr("10 dB")
             << tr("20 dB") << tr("30 dB");
    atten->setComboText(atten_sl);
    atten->setEnabled(false);

    preamp = new ComboEntry(tr("Preamp"));
    QStringList preamp_sl;
    preamp_sl << "Auto" << "Off" << "On";
    preamp->setComboText(preamp_sl);
    preamp->setEnabled(false);

    native_rbw = new CheckBoxEntry("Native RBW");
    rbw = new FreqShiftEntry(tr("RBW"), 0.0);
    vbw = new FreqShiftEntry(tr("VBW"), 0.0);
    auto_rbw = new CheckBoxEntry(tr("Auto RBW"));
    auto_vbw = new CheckBoxEntry(tr("Auto VBW"));

    video_units = new ComboEntry(tr("Video Units"));
    QStringList video_sl;
    video_sl << tr("Log") << tr("Voltage") <<
                tr("Power") << tr("Sample");
    video_units->setComboText(video_sl);

    detector = new ComboEntry(tr("Detector"));
    QStringList detector_sl;
    detector_sl << tr("Min/Max") << tr("Average");
    detector->setComboText(detector_sl);

    sweep_time = new TimeEntry(tr("Swp Time"), Time(0.0), MILLISECOND);

    tg_page->AddWidget(tgSweepSize);
    tg_page->AddWidget(tgSweepType);
    tg_page->AddWidget(tgHighDynamicRange);
    tg_page->AddWidget(tgStoreThru);

    frequency_page->AddWidget(center);
    frequency_page->AddWidget(span);
    frequency_page->AddWidget(start);
    frequency_page->AddWidget(stop);
    frequency_page->AddWidget(step);
    frequency_page->AddWidget(full_zero_span);

    amplitude_page->AddWidget(ref);
    amplitude_page->AddWidget(div);
    amplitude_page->AddWidget(gain);
    amplitude_page->AddWidget(atten);
    amplitude_page->AddWidget(preamp);

    bandwidth_page->AddWidget(native_rbw);
    bandwidth_page->AddWidget(rbw);
    bandwidth_page->AddWidget(vbw);
    bandwidth_page->AddWidget(auto_rbw);
    bandwidth_page->AddWidget(auto_vbw);

    acquisition_page->AddWidget(video_units);
    acquisition_page->AddWidget(detector);
    acquisition_page->AddWidget(sweep_time);

    AppendPage(tg_page);
    AppendPage(frequency_page);
    AppendPage(amplitude_page);
    AppendPage(bandwidth_page);
    AppendPage(acquisition_page);

    // Set panel and connect here
    updatePanel(settings);

    connect(settings, SIGNAL(updated(const SweepSettings*)),
            this, SLOT(updatePanel(const SweepSettings*)));

    connect(tgSweepSize, SIGNAL(valueChanged(double)),
            settings, SLOT(setTgSweepSize(double)));
    connect(tgHighDynamicRange, SIGNAL(clicked(bool)),
            settings, SLOT(setTgHighRange(bool)));
    connect(tgSweepType, SIGNAL(comboIndexChanged(int)),
            settings, SLOT(setTgPassiveDevice(int)));

    connect(center, SIGNAL(freqViewChanged(Frequency)),
            settings, SLOT(setCenter(Frequency)));
    connect(center, SIGNAL(shift(bool)),
            settings, SLOT(increaseCenter(bool)));
    connect(span, SIGNAL(freqViewChanged(Frequency)),
            settings, SLOT(setSpan(Frequency)));
    connect(span, SIGNAL(shift(bool)),
            settings, SLOT(increaseSpan(bool)));
    connect(start, SIGNAL(freqViewChanged(Frequency)),
            settings, SLOT(setStart(Frequency)));
    connect(stop, SIGNAL(freqViewChanged(Frequency)),
            settings, SLOT(setStop(Frequency)));
    connect(step, SIGNAL(freqViewChanged(Frequency)),
            settings, SLOT(setStep(Frequency)));
    connect(full_zero_span, SIGNAL(leftPressed()),
            settings, SLOT(setFullSpan()));
    connect(full_zero_span, SIGNAL(rightPressed()),
            this, SIGNAL(zeroSpanPressed()));

    connect(ref, SIGNAL(amplitudeChanged(Amplitude)),
            settings, SLOT(setRefLevel(Amplitude)));
    connect(ref, SIGNAL(shift(bool)),
            settings, SLOT(shiftRefLevel(bool)));
    connect(div, SIGNAL(valueChanged(double)),
            settings, SLOT(setDiv(double)));
    connect(gain, SIGNAL(comboIndexChanged(int)),
            settings, SLOT(setGain(int)));

    connect(sampleRateRtrl, SIGNAL(comboIndexChanged(int)),
            settings, SLOT(setSampleRate(int)));
    connect(atten, SIGNAL(comboIndexChanged(int)),
            settings, SLOT(setAttenuation(int)));
    connect(preamp, SIGNAL(comboIndexChanged(int)),
            settings, SLOT(setPreAmp(int)));

    connect(native_rbw, SIGNAL(clicked(bool)),
            settings, SLOT(setNativeRBW(bool)));
    connect(rbw, SIGNAL(freqViewChanged(Frequency)),
            settings, SLOT(setRBW(Frequency)));
    connect(rbw, SIGNAL(shift(bool)),
            settings, SLOT(rbwIncrease(bool)));
    connect(vbw, SIGNAL(freqViewChanged(Frequency)),
            settings, SLOT(setVBW(Frequency)));
    connect(vbw, SIGNAL(shift(bool)),
            settings, SLOT(vbwIncrease(bool)));
    connect(auto_rbw, SIGNAL(clicked(bool)), settings, SLOT(setAutoRbw(bool)));
    connect(auto_vbw, SIGNAL(clicked(bool)), settings, SLOT(setAutoVbw(bool)));

    connect(video_units, SIGNAL(comboIndexChanged(int)),
            settings, SLOT(setProcUnits(int)));
    connect(detector, SIGNAL(comboIndexChanged(int)),
            settings, SLOT(setDetector(int)));
    connect(sweep_time, SIGNAL(timeChanged(Time)),
            settings, SLOT(setSweepTime(Time)));
}
void SweepPanel::DeviceConnected(DeviceType type)
{
    native_rbw->Enable(type == DeviceTypeBB60A
                       || type == DeviceTypeBB60C);

    QStringList atten_sl;
    if(type == DeviceTypeSA44A || type == DeviceTypeSA44B) {
        atten_sl << tr("Auto Atten") << tr("0 dB") << tr("5 dB")
                 << tr("10 dB") << tr("15 dB");
    } else {
        atten_sl << tr("Auto Atten") << tr("0 dB") << tr("10 dB")
                 << tr("20 dB") << tr("30 dB");
    }
    atten->setComboText(atten_sl);
}

void SweepPanel::updatePanel(const SweepSettings *settings)
{
    tgSweepSize->SetValue(settings->tgSweepSize);
    tgHighDynamicRange->SetChecked(settings->tgHighRangeSweep);
    tgSweepType->setComboIndex(settings->tgPassiveDevice ? 0 : 1);
    tgStoreThru->RightButton()->setEnabled(settings->tgPassiveDevice);

    center->SetFrequency(settings->Center());
    span->SetFrequency(settings->Span());
    start->SetFrequency(settings->Start());
    stop->SetFrequency(settings->Stop());
    step->SetFrequency(settings->Step());

    ref->SetAmplitude(settings->RefLevel());
    div->SetValue(settings->Div());
    gain->setComboIndex(settings->Gain());
    sampleRateRtrl->setComboIndex(settings->SampleRateRtl());
    atten->setComboIndex(settings->Atten());
    preamp->setComboIndex(settings->Preamp());

    native_rbw->SetChecked(settings->NativeRBW());
    rbw->SetFrequency(settings->RBW());
    vbw->SetFrequency(settings->VBW());
    auto_rbw->SetChecked(settings->AutoRBW());
    auto_vbw->SetChecked(settings->AutoVBW());

    video_units->setComboIndex(settings->ProcessingUnits());
    detector->setComboIndex(settings->Detector());
    sweep_time->SetTime(settings->SweepTime());
}

void SweepPanel::setMode(OperationalMode mode)
{
    bool pagesEnabled = true;
    if(mode == MODE_NETWORK_ANALYZER) {
        pagesEnabled = false;
        PrependPage(tg_page);
    } else {
        RemovePage(tg_page);
    }

    bandwidth_page->SetPageEnabled(pagesEnabled);
    acquisition_page->SetPageEnabled(pagesEnabled);
}

void SweepPanel::storeThrough()
{
    sessionPtr->device->TgStoreThrough();
}

void SweepPanel::storeThroughPad()
{
    sessionPtr->device->TgStoreThroughPad();
}
