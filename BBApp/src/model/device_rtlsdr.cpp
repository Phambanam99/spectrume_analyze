#include "device_rtlsdr.h"
#include <QDebug>
#include <QMessageBox>
#include <cmath>
#include <iostream>
#include "../lib/exceptions.h"
#include "../lib/acquisition.h"
DeviceRtlSdr::DeviceRtlSdr(const Preferences *preferences)
    : Device(preferences), device(nullptr) , last_status(0) {
        device_type = DeviceTypeRtlSdr;
        id = -1;
        open = false;
        serial_number = 0;
        ppm = 0;
        biasT = false;
        gainId = 0;
        directSamplingMode = 0;

        // Handler stuff
        asyncCount = 0;
        sampleRate;

}
bool DeviceRtlSdr::OpenDevice(int dev_index) {
    int num_of_rtls = rtlsdr_get_device_count();
    if (num_of_rtls == 0) {
        throw RPFexception(
            "No RTL-SDR compatible devices found.",
            ReturnValue::NoDeviceFound);
    }

    if (dev_index >= num_of_rtls) {
        throw RPFexception(
            "Invalid RTL device number. Only " + std::to_string(num_of_rtls) + " devices available.",
            ReturnValue::InvalidDeviceIndex);
    }

    int rtl_retval = rtlsdr_open(&device, (uint32_t)dev_index);
    if (rtl_retval < 0 ) {
        throw RPFexception(
            "Could not open rtl_sdr device " + std::to_string(dev_index),
            ReturnValue::HardwareError);
    }
    open = true;
    return true;
}
DeviceRtlSdr::~DeviceRtlSdr() {
    CloseDevice();
}

bool DeviceRtlSdr::OpenDevice() {
    if (rtlsdr_open(&device,0) != 0) {
        qDebug() << "Failed to open RTL-SDR device.";
        last_status = -1;
        open = false;
        return false;
    }
    qDebug() << "RTL-SDR device opened.";
    open = true;
    return true;
}
void static rtlsdrCallback(unsigned char *buf, uint32_t len, void *ctx)
{
}

bool DeviceRtlSdr::Reconfigure(const SweepSettings *s, Trace *t,Params *params) {
    if (!open)  {
            return false;
        }
    const double sampleRates[] = {
        250000,
        1024000,
        1536000,
        1792000,
        1920000,
        2048000,
        2160000,
        2400000,
        2560000,
        2880000,
        3200000
    };

    const char* sampleRatesTxt[] = {
        "250KHz",
        "1.024MHz",
        "1.536MHz",
        "1.792MHz",
        "1.92MHz",
        "2.048MHz",
        "2.16MHz",
        "2.4MHz",
        "2.56MHz",
        "2.88MHz",
        "3.2MHz"
    };
        print_gains();
        int gain = nearest_gain(params -> gain);
    //    std::cerr << "Selected nearest available gain: " << gain
    //              << " (" << 0.1*gain << " dB)" << std::endl;
        set_gain(gain);

        try {
           set_frequency(params -> cfreq);
        }
        catch (RPFexception&) {}

        // Set frequency correction
        if (params ->ppm_error != 0) {
            set_freq_correction(params -> ppm_error);
    //        std::cerr << "PPM error set to: " << params.ppm_error << std::endl;
        }

        // Set sample rate
        set_sample_rate(params -> sample_rate);
        t->SetSettings(*s);
        t->SetSize(params -> N); // Example size
        t->SetFreq(s->Span() / 1024, s->Center() - s->Span() / 2);
        t->SetUpdateRange(0, 1024);

        // No direct function to get diagnostics in RTL-SDR
        last_temp = 0;
        voltage = 0;
        current = 0;
        qDebug() << "Reconfig rtlsdr";
    return true;
}



bool DeviceRtlSdr::OpenDeviceWithSerial(int serialToOpen) {
        rtlsdr_dev_t* dev = nullptr;
    // Get the count of available RTL-SDR devices
        int deviceCount = rtlsdr_get_device_count();
        if (deviceCount == 0) {
            qDebug() << "No RTL-SDR devices found!";
            open = false;
            return false;

        }
        if (rtlsdr_open(&dev, 0) < 0) {
            qDebug() << "Failed to open RTL-SDR device";
            open = false;
            return false;
        }

        device = dev;
         qDebug() << "Successfully opened RTL-SDR device " << device ;
        open = true;
        return true;
}
bool DeviceRtlSdr::GetSweep(const SweepSettings *s, Trace *t)
{
          return true;
}
bool DeviceRtlSdr::CloseDevice() {
    if (device) {
        rtlsdr_close(device);
        device = nullptr;
        qDebug() << "RTL-SDR device closed.";
        open = false;
        return true;
    }
      open = true;
    return false;


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
//    center_freq = freq;
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
//    sample_rate = rate;
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
//    gain = gain_value;
//    qDebug() << "Gain set to:" << gain;
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
std::vector<int> DeviceRtlSdr::gains() const {
    int number_of_gains = rtlsdr_get_tuner_gains(device, NULL);
    if (number_of_gains <= 0) {
        throw RPFexception(
            "RTL device: could not read the number of available gains.",
            ReturnValue::HardwareError);
    }
    std::vector<int> gains(number_of_gains);
    if (rtlsdr_get_tuner_gains(device, gains.data()) <= 0) {
        throw RPFexception(
            "RTL device: could not retrieve gain values.",
            ReturnValue::HardwareError);
    }
    return gains;
}

uint32_t DeviceRtlSdr::sample_rate() const {
    uint32_t sample_rate = rtlsdr_get_sample_rate(device);
    if (sample_rate == 0) {
        throw RPFexception(
            "RTL device: could not read sample rate.",
            ReturnValue::HardwareError);
    }
    return sample_rate;
}

uint32_t DeviceRtlSdr::frequency() const {
    uint32_t frequency = rtlsdr_get_center_freq(device);
    if (frequency == 0) {
        throw RPFexception(
            "RTL device: could not read frequency.",
            ReturnValue::HardwareError);
    }
    return frequency;
}

bool DeviceRtlSdr::read(std::vector<uint8_t>& buffer) const {
    int n_read;
    rtlsdr_reset_buffer(device);
    rtlsdr_read_sync(device, buffer.data(), buffer.size(), &n_read);
    return n_read == (signed)buffer.size();
}

void DeviceRtlSdr::set_gain(int gain) {
    int status = 0;
    status += rtlsdr_set_tuner_gain_mode(device, 1);
    status += rtlsdr_set_tuner_gain(device, gain);

    if (status != 0) {
        throw RPFexception(
            "RTL device: could not set gain.",
            ReturnValue::HardwareError);
    }
}

void DeviceRtlSdr::set_frequency(uint32_t frequency) {
    if (rtlsdr_set_center_freq(device, frequency) < 0) {
        throw RPFexception(
            "RTL device: could not set center frequency.",
            ReturnValue::HardwareError);
    }
    // This sleeping is inherited from other code. There have been hints of strange
    // behaviour if it was commented out, so we left it in. If you actually know
    // why this would be necessary (or, to the contrary, that it is complete
    // bullshit), you are most welcome to explain it here!
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
}

void DeviceRtlSdr::set_freq_correction(int ppm_error) {
    if (rtlsdr_set_freq_correction(device, ppm_error) < 0) {
        throw RPFexception(
            "RTL device: could not set frequency correction.",
            ReturnValue::HardwareError);
    }
}

void DeviceRtlSdr::set_sample_rate(uint32_t sample_rate) {
    if (rtlsdr_set_sample_rate(device, sample_rate)) {
        throw RPFexception(
            "RTL device: could not set sample rate.",
            ReturnValue::HardwareError);
    }
}

int DeviceRtlSdr::nearest_gain(int gain) const {
    int dif = std::numeric_limits<int>::max();
    int selected = 0;
    for (const auto& trial_gain : gains()) {
        int temp = abs(trial_gain - gain);
        if ( temp < dif ) {
            dif = temp;
            selected = trial_gain;
        }
    }
    return selected;
}

void DeviceRtlSdr::print_gains() const {
    auto gain_table = gains();

    std::cerr << "Available gains (in 1/10th of dB): ";
    for (unsigned int i = 0; i < gain_table.size(); i++) {
        if (i != 0)
            std::cerr << ", ";
        std::cerr << gain_table[i];
    }
    std::cerr << std::endl;
}
QStringList DeviceRtlSdr::qstring_gains() const {

    auto gain_table = gains();

    QStringList stringList;
    for (unsigned int i = 0; i < gain_table.size(); i++) {
        if (i != 0){
         stringList.append(QString::number(gain_table[i]));
        }
    }
     return stringList;
}
QString DeviceRtlSdr::GetDeviceString() const {
    return QString("RTL-SDR Device");
}


const char* DeviceRtlSdr::GetLastStatusString() const {
    return "Status: OK";
}
