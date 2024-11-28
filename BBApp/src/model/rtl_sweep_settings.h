#ifndef RTL_SWEEP_SETTINGS_H
#define RTL_SWEEP_SETTINGS_H


#include <QObject>
#include <QSettings>
#include "../lib/rtlsdr.h"
#include "../lib/frequency.h"
#include "../lib/amplitude.h"
#include "../lib/time_type.h"
#include "../model/device_rtlsdr.h"
class RTL_SDR_Settings : public QObject {
    Q_OBJECT

public:
    RTL_SDR_Settings();
    RTL_SDR_Settings(const RTL_SDR_Settings &other);
    ~RTL_SDR_Settings();

    RTL_SDR_Settings& operator=(const RTL_SDR_Settings &other);
    bool operator==(const RTL_SDR_Settings &other) const;
    bool operator!=(const RTL_SDR_Settings &other) const;

    virtual void LoadDefaults();
    bool Load(QSettings &s);
    bool Save(QSettings &s) const;
    void setDevice(DeviceRtlSdr* myDevice){  device = myDevice;}
    Frequency Start() const { return start; }
    Frequency Stop() const { return stop; }
    Frequency Center() const { return center; }
    Frequency Span() const { return span; }
    Frequency Step() const { return step; }
    Frequency RBW() const { return rbw; }
    Frequency VBW() const { return vbw; }

    Amplitude RefLevel() const { return refLevel; }
    double Div() const { return div; }
    int Atten() const { return attenuation; }
    int Gain() const { return gain; }

    Time SweepTime() const { return sweepTime; }
    int Detector() const { return detector; }
    bool Rejection() const { return rejection; }

    void EmitUpdated() { emit updated(this); }

protected:
    virtual void UpdateProgram();

private:
    DeviceRtlSdr* device;  // RTL-SDR device handle
    Frequency start;
    Frequency stop;
    Frequency center;
    Frequency span;
    Frequency step;
    Frequency rbw;
    Frequency vbw;

    Amplitude refLevel;
    double div;
    int attenuation;
    int gain;

    Time sweepTime;
    int detector;
    bool rejection;

signals:
    void updated(const RTL_SDR_Settings*);

public slots:
    void setStart(Frequency f);
    void setStop(Frequency f);
    void setCenter(Frequency f);
    void setStep(Frequency f);
    void setSpan(Frequency f);
    void setRBW(Frequency f);
    void setVBW(Frequency f);
    void setRefLevel(Amplitude f);
    void setAttenuation(int atten_ix);
    void setGain(int gain_ix);
    void setSweepTime(Time new_sweep_time);
    void setDetector(int new_detector);
    void setRejection(bool image_reject);
    void handleDeviceSelection(int index);

};

#endif // RTL_SWEEP_SETTINGS_H
