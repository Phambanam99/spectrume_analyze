#include "device_rtlsdr.h"
#include <QDebug>
#include <QMessageBox>
DeviceRtlSdr::DeviceRtlSdr(const Preferences *preferences)
    : Device(preferences), device(nullptr), center_freq(100000000), sample_rate(2048000), gain(0), last_status(0) {
    device_type = DeviceTypeRtlSdr;
}

DeviceRtlSdr::~DeviceRtlSdr() {
    CloseDevice();
}

bool DeviceRtlSdr::OpenDevice() {
    if (rtlsdr_open(&device, 0) != 0) {
        qDebug() << "Failed to open RTL-SDR device.";
        last_status = -1;
        return false;
    }
    qDebug() << "RTL-SDR device opened.";
    return true;
}

bool DeviceRtlSdr::OpenDeviceWithSerial(int serialToOpen) {
        rtlsdr_dev_t* dev = nullptr;
    // Get the count of available RTL-SDR devices
        int deviceCount = rtlsdr_get_device_count();
        if (deviceCount == 0) {
            qDebug() << "No RTL-SDR devices found!";
            return false;
        }

        // Iterate over all devices to find the one with the matching serial number
        for (int i = 0; i < deviceCount; ++i) {
            char manufacturer[256];
            char product[256];
            char serial[256];

            // Retrieve the device's information (serial number, manufacturer, etc.)
            if (rtlsdr_get_device_usb_strings(i, manufacturer, product, serial) < 0) {
                qDebug() << "Failed to get device strings for device " << i;
                continue;
            }

            // Check if the serial number matches the one we're looking for
            if (QString(serial) == QString::number(serialToOpen)) {
                qDebug() << "Found matching device with serial: " << serial;

                // Open the device
                if (rtlsdr_open(&dev, i) < 0) {
                    qDebug() << "Failed to open RTL-SDR device with serial: " << serial;
                    return false;
                }

                qDebug() << "Successfully opened RTL-SDR device with serial: " << serial;
                return true;
            }
        }

        // If no matching device is found, show an error message
        QMessageBox::warning(nullptr, "Device Not Found", "No device with the specified serial number was found.");
        return false;
}

bool DeviceRtlSdr::CloseDevice() {
    if (device) {
        rtlsdr_close(device);
        device = nullptr;
        qDebug() << "RTL-SDR device closed.";
    }
    return true;
}

bool DeviceRtlSdr::Abort() {
    if (!device) return false;
    rtlsdr_reset_buffer(device);
    qDebug() << "Buffer reset.";
    return true;
}

bool DeviceRtlSdr::Preset() {
    // Reset frequency, sample rate, and gain to default
    return ConfigureFrequency(100000000) && ConfigureSampleRate(2048000) && ConfigureGain(0);
}

bool DeviceRtlSdr::ConfigureFrequency(uint32_t freq) {
    if (!device) return false;
    if (rtlsdr_set_center_freq(device, freq) != 0) {
        qDebug() << "Failed to set center frequency.";
        last_status = -1;
        return false;
    }
    center_freq = freq;
    qDebug() << "Center frequency set to:" << freq;
    return true;
}

bool DeviceRtlSdr::ConfigureSampleRate(uint32_t rate) {
    if (!device) return false;
    if (rtlsdr_set_sample_rate(device, rate) != 0) {
        qDebug() << "Failed to set sample rate.";
        last_status = -1;
        return false;
    }
    sample_rate = rate;
    qDebug() << "Sample rate set to:" << rate;
    return true;
}

bool DeviceRtlSdr::ConfigureGain(int gain_value) {
    if (!device) return false;
    if (rtlsdr_set_tuner_gain_mode(device, gain_value == 0 ? 0 : 1) != 0) {
        qDebug() << "Failed to configure gain mode.";
        last_status = -1;
        return false;
    }
    if (gain_value > 0) {
        if (rtlsdr_set_tuner_gain(device, gain_value) != 0) {
            qDebug() << "Failed to set tuner gain.";
            last_status = -1;
            return false;
        }
    }
    gain = gain_value;
    qDebug() << "Gain set to:" << gain;
    return true;
}

bool DeviceRtlSdr::GetIQData(std::vector<uint8_t> &iq_buffer, int &n_read) {
    if (!device) return false;

    iq_buffer.resize(16384); // Adjust buffer size as needed
    if (rtlsdr_read_sync(device, iq_buffer.data(), iq_buffer.size(), &n_read) != 0) {
        qDebug() << "Failed to read IQ data.";
        last_status = -1;
        return false;
    }
    qDebug() << "Read IQ data size:" << n_read;
    return true;
}

QString DeviceRtlSdr::GetDeviceString() const {
    return QString("RTL-SDR Device");
}


const char* DeviceRtlSdr::GetLastStatusString() const {
    return "Status: OK";
}
