#ifndef DEVICE_RTLSDR_H
#define DEVICE_RTLSDR_H

#include "device.h"
#include "../lib/device_traits.h"

class Preferences;

class DeviceRtlSdr : public Device {
public:
    DeviceRtlSdr(const Preferences *preferences);
    virtual ~DeviceRtlSdr();
    DeviceRtlSdr();
    // Device control
    virtual bool OpenDevice();
    virtual bool OpenDeviceWithSerial(int serialToOpen);
    virtual bool CloseDevice();
    virtual bool Abort();
    virtual bool Preset();

    // Configuration
    virtual bool ConfigureFrequency(uint32_t center_freq);
    virtual bool ConfigureSampleRate(uint32_t sample_rate);
    virtual bool ConfigureGain(int gain);

    // Data fetching
    virtual bool GetIQData(std::vector<uint8_t> &iq_buffer, int &n_read);

    // Status and info
    virtual QString GetDeviceString() const;
    virtual const char* GetLastStatusString() const;

private:

    rtlsdr_dev_t *device;
    uint32_t center_freq;
    uint32_t sample_rate;
    int gain;
    int last_status;

private:
    DISALLOW_COPY_AND_ASSIGN(DeviceRtlSdr)
};

#endif // DEVICE_RTLSDR_H
