#ifndef DEVICE_RTLSDR_H
#define DEVICE_RTLSDR_H

#include "device.h"
#include "../lib/device_traits.h"
#include "../lib/fftw3.h"
class Preferences;

class DeviceRtlSdr : public Device {
public:
    DeviceRtlSdr(const Preferences *preferences);
    virtual ~DeviceRtlSdr();
     DeviceRtlSdr();
    // Device control
    virtual bool OpenDevice(int index);
    bool OpenDevice();
    virtual bool OpenDeviceWithSerial(int serialToOpen);
    virtual bool CloseDevice();
    virtual bool Abort();
    virtual bool Preset();
    void computeFFT(Trace *t);
    void runReceiver();
    // Configuration
    virtual bool ConfigureFrequency(uint32_t center_freq);
    virtual bool ConfigureSampleRate(uint32_t sample_rate);
    virtual bool ConfigureGain(int gain);
    // Data fetching
    virtual bool GetIQData(std::vector<uint8_t> &iq_buffer, int &n_read);
    virtual bool Reconfigure(const SweepSettings *s, Trace *t);
    virtual bool GetSweep(const SweepSettings *s, Trace *t);
    // Status and info

    virtual QString GetDeviceString() const;
    void worker(Trace *t);
    virtual const char* GetLastStatusString() const;

    // Reading parameters & data.
    std::vector<int> gains() const;
    uint32_t sample_rate() const;
    uint32_t frequency() const ;
    bool read(std::vector<uint8_t>& buffer) const;

    // Parameter setting.
    void set_gain(int gain);
    void set_frequency(uint32_t frequency);
    void set_freq_correction(int ppm_error);
    void set_sample_rate(uint32_t sample_rate);

    // Convenience functions.
    int nearest_gain(int gain) const;
    void print_gains() const;
private:

    rtlsdr_dev_t *device;
    int last_status;
    bool running;
    QThread *workerThread;
    int ppm ;
    std::vector<int> spectrumData;  // Dữ liệu phổ
    bool hasData;
    std::vector<float> fftMagnitude; // Kết quả FFT// Cờ để kiểm tra xem có dữ liệu hay không
    bool biasT ;

    int gainId;
    std::vector<int> gainList;

    bool rtlAgc ;
    bool tunerAgc ;
    bool offsetTuning;

    int directSamplingMode ;

    // Handler stuff
    int asyncCount;
    int sampleRate;
    char dbTxt[128];
private:
    DISALLOW_COPY_AND_ASSIGN(DeviceRtlSdr)
};

#endif // DEVICE_RTLSDR_H
