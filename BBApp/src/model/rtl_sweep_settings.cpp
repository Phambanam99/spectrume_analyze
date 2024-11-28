#include "rtl_sweep_settings.h"

#include <QSettings>
#include "../lib/rtlsdr.h"
#include <QDebug>
RTL_SDR_Settings::RTL_SDR_Settings() : device(nullptr) {
    LoadDefaults();
}

RTL_SDR_Settings::~RTL_SDR_Settings() {
    if (device) {
        device->CloseDevice();
    }
}

RTL_SDR_Settings::RTL_SDR_Settings(const RTL_SDR_Settings &other) {
    *this = other;
}

RTL_SDR_Settings& RTL_SDR_Settings::operator=(const RTL_SDR_Settings &other) {
    if (this == &other) return *this;

    start = other.start;
    stop = other.stop;
    center = other.center;
    span = other.span;
    step = other.step;
    rbw = other.rbw;
    vbw = other.vbw;

    refLevel = other.refLevel;
    div = other.div;
    attenuation = other.attenuation;
    gain = other.gain;

    sweepTime = other.sweepTime;
    detector = other.detector;
    rejection = other.rejection;

    return *this;
}

bool RTL_SDR_Settings::operator==(const RTL_SDR_Settings &other) const {
    return start == other.start && stop == other.stop && center == other.center &&
           span == other.span && step == other.step && rbw == other.rbw && vbw == other.vbw &&
           refLevel == other.refLevel && div == other.div && attenuation == other.attenuation &&
           gain == other.gain && sweepTime == other.sweepTime && detector == other.detector &&
           rejection == other.rejection;
}

bool RTL_SDR_Settings::operator!=(const RTL_SDR_Settings &other) const {
    return !(*this == other);
}

void RTL_SDR_Settings::LoadDefaults() {
    start = Frequency(10000000);  // Start frequency
    stop = Frequency(1000000000); // Stop frequency
    center = (start + stop) / 2.0;
    span = stop - start;
    step = Frequency(20000000);   // Frequency step size
    rbw = Frequency(100000);      // Resolution bandwidth
    vbw = Frequency(30000);       // Video bandwidth

    refLevel = Amplitude(-30.0, DBM);
    div = 10.0;
    attenuation = 0;
    gain = 0;

    sweepTime = Time(0.001);
    detector = 0; // Example detector
    rejection = false;

//    // Initialize RTL-SDR device
//    if (rtlsdr_open(&device, 0) < 0) {
//        qDebug() << "Failed to open RTL-SDR device";
//    } else {
//        qDebug() << "Opened device rtl-sdr";
//    }
}

bool RTL_SDR_Settings::Load(QSettings &s) {
    start = Frequency(s.value("Sweep/Start", start.Val()).toDouble());
    stop = Frequency(s.value("Sweep/Stop", stop.Val()).toDouble());
    center = Frequency(s.value("Sweep/Center", center.Val()).toDouble());
    span = Frequency(s.value("Sweep/Span", span.Val()).toDouble());
    step = Frequency(s.value("Sweep/Step", step.Val()).toDouble());
    rbw = Frequency(s.value("Sweep/RBW", rbw.Val()).toDouble());
    vbw = Frequency(s.value("Sweep/VBW", vbw.Val()).toDouble());

    refLevel.Load(s, "Sweep/RefLevel");
    div = s.value("Sweep/Division", div).toDouble();
    attenuation = s.value("Sweep/Attenuation", attenuation).toInt();
    gain = s.value("Sweep/Gain", gain).toInt();

    sweepTime = Time(s.value("Sweep/SweepTime", sweepTime.Val()).toDouble());
    detector = s.value("Sweep/Detector", detector).toInt();
    rejection = s.value("Sweep/Rejection", rejection).toBool();

    return true;
}

bool RTL_SDR_Settings::Save(QSettings &s) const {
    s.setValue("Sweep/Start", start.Val());
    s.setValue("Sweep/Stop", stop.Val());
    s.setValue("Sweep/Center", center.Val());
    s.setValue("Sweep/Span", span.Val());
    s.setValue("Sweep/Step", step.Val());
    s.setValue("Sweep/RBW", rbw.Val());
    s.setValue("Sweep/VBW", vbw.Val());

    refLevel.Save(s, "Sweep/RefLevel");
    s.setValue("Sweep/Division", div);
    s.setValue("Sweep/Attenuation", attenuation);
    s.setValue("Sweep/Gain", gain);

    s.setValue("Sweep/SweepTime", sweepTime.Val());
    s.setValue("Sweep/Detector", detector);
    s.setValue("Sweep/Rejection", rejection);

    return true;
}

void RTL_SDR_Settings::UpdateProgram() {
    // Implement any updates related to frequency, bandwidth, etc.
    // RTL-SDR specific configuration might be needed here.
    emit updated(this);
}

// Slot implementations

void RTL_SDR_Settings::setStart(Frequency f) {
    start = f;
    span = stop - start;
    center = (start + stop) / 2.0;
    UpdateProgram();
}

void RTL_SDR_Settings::setStop(Frequency f) {
    stop = f;
    span = stop - start;
    center = (start + stop) / 2.0;
    UpdateProgram();
}

void RTL_SDR_Settings::setCenter(Frequency f) {
    center = f;
    span = stop - start;
    start = center - (span / 2.0);
    stop = center + (span / 2.0);
    UpdateProgram();
}

void RTL_SDR_Settings::setStep(Frequency f) {
    step = f;
    UpdateProgram();
}

void RTL_SDR_Settings::setSpan(Frequency f) {
    span = f;
    stop = start + span;
    center = (start + stop) / 2.0;
    UpdateProgram();
}

void RTL_SDR_Settings::setRBW(Frequency f) {
    rbw = f;
    UpdateProgram();
}

void RTL_SDR_Settings::setVBW(Frequency f) {
    vbw = f;
    UpdateProgram();
}

void RTL_SDR_Settings::setRefLevel(Amplitude f) {
    refLevel = f;
    UpdateProgram();
}

void RTL_SDR_Settings::setAttenuation(int atten_ix) {
    attenuation = atten_ix;
    UpdateProgram();
}

void RTL_SDR_Settings::setGain(int gain_ix) {
    gain = gain_ix;
    UpdateProgram();
}

void RTL_SDR_Settings::setSweepTime(Time new_sweep_time) {
    sweepTime = new_sweep_time;
    UpdateProgram();
}

void RTL_SDR_Settings::setDetector(int new_detector) {
    detector = new_detector;
    UpdateProgram();
}

void RTL_SDR_Settings::setRejection(bool image_reject) {
    rejection = image_reject;
    UpdateProgram();
}
void RTL_SDR_Settings::handleDeviceSelection(int index) {
        if (index == 0) {
           qDebug() << "No device selected.";
           if(device -> CloseDevice())
           {
               return;
           }
           else {
             qDebug() << "Cannot close device.";
           }
       }
       // Handle opening the RTL-SDR device based on the selected index
       qDebug() << "Selected device index:" << index;
        // Open the first RTL-SDR device
       if (device ->OpenDeviceWithSerial(index)) {
           qDebug() << "Device opened successfully!" << index;
       }
   }

