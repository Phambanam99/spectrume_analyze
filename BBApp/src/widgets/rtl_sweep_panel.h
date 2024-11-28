#ifndef RTL_SWEEP_PANEL_H
#define RTL_SWEEP_PANEL_H

#include "dock_panel.h"
#include "entry_widgets.h"

class RTL_SDR_Settings;
class Session;
class Device;
class RtlSweepPanel : public DockPanel {
    Q_OBJECT

    Session *sessionPtr; // Does not own

public:
    RtlSweepPanel(const QString &title, QWidget *parent, Session *session);
    ~RtlSweepPanel();

private:
    QComboBox *deviceSelected;
    DockPage *frequency_page;
    FrequencyEntry *start;
    FrequencyEntry *stop;
    FrequencyEntry *step;

public slots:
    void updatePanel(const RTL_SDR_Settings *settings);

private:
    DISALLOW_COPY_AND_ASSIGN(RtlSweepPanel)
};

#endif // RTL_SWEEP_PANEL_H
