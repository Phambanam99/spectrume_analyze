#include "rtl_sweep_panel.h"
#include "model/session.h"

RtlSweepPanel::RtlSweepPanel(const QString &title,
                       QWidget *parent,
                       Session *session)
    : DockPanel(title, parent), sessionPtr(session) {
    const RTL_SDR_Settings *settings = session->rtl_sweep_settings;

    // Frequency page
    frequency_page = new DockPage(tr("Frequency"));

    start = new FrequencyEntry(tr("Start"), 0.0);
    stop = new FrequencyEntry(tr("Stop"), 0.0);
    step = new FrequencyEntry(tr("Step"), 0.0);

    frequency_page->AddWidget(start);
    frequency_page->AddWidget(stop);
    frequency_page->AddWidget(step);

    AppendPage(frequency_page);

    // Set panel and connect signals
    updatePanel(settings);

    connect(start, SIGNAL(freqViewChanged(Frequency)),
            settings, SLOT(setStart(Frequency)));
    connect(stop, SIGNAL(freqViewChanged(Frequency)),
            settings, SLOT(setStop(Frequency)));
    connect(step, SIGNAL(freqViewChanged(Frequency)),
            settings, SLOT(setStep(Frequency)));
}

RtlSweepPanel::~RtlSweepPanel() {
    delete frequency_page;
}

void RtlSweepPanel::updatePanel(const RTL_SDR_Settings *settings) {
    start->SetFrequency(settings->Start());
    stop->SetFrequency(settings->Stop());
    step->SetFrequency(settings->Step());
}
