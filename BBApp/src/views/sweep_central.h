#ifndef SWEEP_CENTRAL_H
#define SWEEP_CENTRAL_H

#include <thread>
#include <atomic>

#include "views/central_stack.h"
#include "../model/session.h"
#include "../widgets/entry_widgets.h"
#include "../model/datastore.h"
#include "../lib/params.h"
#include "../lib/acquisition.h"
#include <memory>
class QToolBar;
class PlaybackToolBar;
class QSplitter;
class TraceView;
class WaterfallView;



class SweepCentral : public CentralWidget {
    Q_OBJECT

public:
    SweepCentral(Session *sPtr,
                 QToolBar *mainToolBar,
                 QWidget *parent = 0,
                 Qt::WindowFlags f = 0);
    ~SweepCentral();

    void StartStreaming();
    void StopStreaming();
    void ResetView();
    bool runReceiver_(Trace *trace, Params params, Plan *plan,
                      DeviceRtlSdr *rtldev, AuxData *auxData, Datastore *data);
    void GetViewImage(QImage &image);
    void ReconfiugreRtl(DeviceRtlSdr *rtldev);
    Frequency GetCurrentCenterFreq() const {
        return session_ptr->sweep_settings->Center(); }
    // Force view back to initial start-up values
    // No persistence, waterfall, etc.
    // Getter methods to access configured objects

protected:
    void resizeEvent(QResizeEvent*);
    void keyPressEvent(QKeyEvent*);
    void wheelEvent(QWheelEvent*);

private:
    void Reconfigure();
    void SweepThread();
    void SweepThreadRtl();
    void SweepThreadSignalHound();
    void PlaybackThread();

    bool reconfigure;
    Trace trace;
    RealTimeFrame rtFrame;
    SweepSettings last_config; // Last known working settings

    TraceView *trace_view;

    ComboBox *waterfall_combo;
    // Line persistence
    QCheckBox *persistence_check;
    SHPushButton *persistence_clear;
    // Real-time persistence
    QCheckBox *realTimePersistenceCheck;
    QSlider *intensitySlider;
    QList<QAction*> sweepOnlyActions, realTimeActions;

    PlaybackToolBar *playback;

    // Copy of session
    Session *session_ptr;

    std::thread thread_handle;
    bool programClosing;
    std::atomic<bool> sweeping;
    std::atomic<int> sweep_count;
    Params params;                     // Configuration parameters
signals:
    void updateView();

public slots:
    void changeMode(int newState);
    void settingsChanged(const SweepSettings *ss);

//    void storeThru() { session_ptr->device->TgStoreThrough(true); }
//    void storeThroughPad() { session_ptr->device->TgStoreThrough(false); }

private slots:
    void singleSweepPressed();
    void continuousSweepPressed();
    // Update the view behind the scenes
    void forceUpdateView();
    void playFromFile(bool play);

};

#endif // SWEEP_CENTRAL_H
