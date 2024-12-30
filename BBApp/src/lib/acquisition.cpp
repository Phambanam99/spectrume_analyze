
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <thread>
#include "acquisition.h"
#include "../model/datastore.h"
#include "metadata.h"
#include <math.h>
#include "../model/device_rtlsdr.h"
#include <QTime>
std::vector<float> AuxData::calculateWindowFunction(int N, const std::string& type) {
    std::vector<float> window(N);
     const double M_PI  =3.141592653589793238463;
    if (type == "hamming") {
        for (int n = 0; n < N; n++) {
            window[n] = 0.54 - 0.46 * std::cos(2 * M_PI * n / (N - 1));
        }
    } else if (type == "hann") {
        for (int n = 0; n < N; n++) {
            window[n] = 0.5 * (1 - std::cos(2 * M_PI * n / (N - 1)));
        }
    } else if (type == "blackman") {
        for (int n = 0; n < N; n++) {
            window[n] = 0.42 - 0.5 * std::cos(2 * M_PI * n / (N - 1)) + 0.08 * std::cos(4 * M_PI * n / (N - 1));
        }
    } else {
        // Rectangular (mặc định không thay đổi tín hiệu)
        for (int n = 0; n < N; n++) {
            window[n] = 1.0;
        }
    }

    return window;
}
AuxData::AuxData(const Params& params) {
    // Hardcode giá trị của window function và baseline

    // 1. Tạo window function (ví dụ sử dụng Hamming window)
    window_values = calculateWindowFunction(params.N, "hamming");
    if ((int)window_values.size() == params.N) {
        std::cerr << "Succesfully generated " << window_values.size() << " window function points." << std::endl;
    } else {
        throw RPFexception(
            "Error generating window function. Expected " + std::to_string(params.N)
                + " values, found " + std::to_string(window_values.size()) + ".",
            ReturnValue::InvalidInput);
    }

    baseline_values = std::vector<double>(params.N, 30); // Gán tất cả giá trị baseline là -20.0

    if ((int)baseline_values.size() == params.N) {
        std::cerr << "Succesfully generated " << baseline_values.size() << " baseline points." << std::endl;
    } else {
        throw RPFexception(
            "Error generating baseline values. Expected " + std::to_string(params.N)
                + " values, found " + std::to_string(baseline_values.size()) + ".",
            ReturnValue::InvalidInput);
    }
}

Plan::Plan(Params& params_, int actual_samplerate_) :
    actual_samplerate(actual_samplerate_), params(params_)
{
    double min_overhang;

    // Calculate the number of repeats according to the true sample rate.
    if (params.integration_time_isSet)
        params.repeats = ceil(actual_samplerate * params.integration_time / params.N);

    // Adjust buffer size
    if (!params.buf_length_isSet) {
        int64_t base_buf_multiplier = ceil((2.0 * params.N * params.repeats) / base_buf);
        // If less than approximately 1.6 MB of data is needed, make the buffer the
        // smallest possible while still keeping the size to a multiple of
        // base_buf. Otherwise, set it to 100 * base_buf.
        // If you know what should fit your purposes well, feel free to use the
        // command line options to override this.
        if (base_buf_multiplier <= default_buf_multiplier) {
            params.buf_length = base_buf * ((base_buf_multiplier == 0 ) ? 1 : base_buf_multiplier);
        }
    }

    /*
    the crop percentage will be used to:
      - in function write_data(): to exclude from output half of (cropPercentage * bins) on each side of the FFT
      - in this function: only if hops are performed: calculate a frequency offset by which the center frequency
        of each hop scan should be lowered in order to produce a continuous and non-overlapping
        series of bins for the overall frequency range
  */
    // Make a plan of frequency hopping.
    // We're stuffing a vector full of frequencies that we wish to eventually tune to.
    if (params.freq_hopping_isSet) {
        min_overhang = actual_samplerate*params.min_overlap/100;

        if( params.cropPercentage > 0 )
        {
            // since overlap and crop are mutually exclusive options,
            // I'm reusing the min_overhang variable for the crop frequency offset mentioned above
            if( double(params.stopfreq - params.startfreq) > double(actual_samplerate) )
            {
                // we shift down frequency only if we are hopping:
                min_overhang = actual_samplerate*params.cropPercentage/100;
                cropFreqOffset = min_overhang;  // will bring this out as metadata
            }
            else
            {
                min_overhang = 0;
                cropFreqOffset = min_overhang;  // will bring this out as metadata
            }
        }

        hops = ceil((double(params.stopfreq - params.startfreq) - min_overhang) / (double(actual_samplerate) - min_overhang));
          std::cerr << "Planned hops: " << hops << std::endl;
        if (hops > 1) {
            int overhang = (hops*actual_samplerate - (params.stopfreq - params.startfreq)) / (hops - 1);
            freqs_to_tune.push_back(params.startfreq + actual_samplerate/2.0);
            //Mmmm, thirsty? waah-waaah...
            for (int hop = 1; hop < hops; hop++) {
                freqs_to_tune.push_back(freqs_to_tune.back() + actual_samplerate - overhang);
            }
        }
        else
            freqs_to_tune.push_back((params.startfreq + params.stopfreq)/2);
    }
    // If there is only one hop, no problem.
    else {
        freqs_to_tune.push_back(params.cfreq);
    }
//    print();
}

void Plan::print() const {
    std::cerr << "Planned hops: " << hops << std::endl;
    std::cerr << "Number of bins: " << params.N << std::endl;
    std::cerr << "Total number of (complex) samples to collect: " << (int64_t)params.N*params.repeats << std::endl;
    std::cerr << "Buffer length: " << params.buf_length << std::endl;
    std::cerr << "Number of averaged spectra: " << params.repeats << std::endl;
    std::cerr << "actual_samplerate " << actual_samplerate << std::endl;;
    std::cerr << "Estimated time of measurements: " << (double)params.N * params.repeats / actual_samplerate << " seconds" << std::endl;
    if (params.strict_time)
        std::cerr << "Acquisition will unconditionally terminate after " << params.integration_time << " seconds." << std::endl;
}


Acquisition::Acquisition(const Params& params_,
                         AuxData& aux_,
                         DeviceRtlSdr& rtldev_,
                         Datastore& data_,
                         int actual_samplerate_,
                         int freq_) :
    params(params_), aux(aux_), rtldev(rtldev_), data(data_),
    actual_samplerate(actual_samplerate_), freq(freq_)
{
    deviceReadouts =0;
    successfulReadouts = 0;
}

#include <vector>
#include <cmath>

#include <iostream>
void Acquisition::runReceiver_()
{
}

void Acquisition::run() {
    std::fill(data.pwr.begin(), data.pwr.end(), 0);
    data.acquisition_finished = false;
    data.repeats_done = 0;

    //std::thread t(&Datastore::fftThread, std::ref(data));
    std::thread t(&Datastore::fftThread, &data);

    std::unique_lock<std::mutex>
        status_lock(data.status_mutex, std::defer_lock);
    int64_t dataTotal = 2 * params.N * params.repeats;
    int64_t dataRead = 0;

    while (dataRead < dataTotal) {
        // Wait until a buffer is empty
        status_lock.lock();
        data.queue_histogram[data.empty_buffers.size()]++;
        while (data.empty_buffers.empty())
            data.status_change.wait(status_lock);

        std::vector<uint8_t>& buffer(*data.empty_buffers.front());
        data.empty_buffers.pop_front();
        status_lock.unlock();

        // Figure out how much data to read.
        int64_t dataNeeded = dataTotal - dataRead;
        if (dataNeeded >= params.buf_length)
            // More than one bufferful of data needed. Leave the rest for later.
            dataNeeded = params.buf_length;
        else {
            // Less than one whole buffer needed. Round the number of (real)
            // samples upwards to the next multiple of base_buf.
            dataNeeded = base_buf * ceil((double)dataNeeded / base_buf);
            if (dataNeeded > params.buf_length) {
                // Nope, too much. We'll still have to do this in two readouts.
                dataNeeded = params.buf_length;
            }
        }
        // Resize the buffer to match the needed amount of data.
        buffer.resize(dataNeeded);

        bool read_success = rtldev.read(buffer);
        deviceReadouts++;

        if (!read_success) {
            std::cerr << "Error: dropped samples." << std::endl;
            // There is effectively no data in this buffer - consider it empty.
            status_lock.lock();
            // Push the buffer to the front of the queue because it already has
            // the correct size and we'll just pop it again on next iteration.
            data.empty_buffers.push_front(&buffer);
            status_lock.unlock();
            // No need to notify the worker thread in this case.
        }
        else {
            successfulReadouts++;
            dataRead += dataNeeded;
            status_lock.lock();
            data.occupied_buffers.push_back(&buffer);
            data.status_change.notify_all();
            status_lock.unlock();
        }

    }

    status_lock.lock();
    data.acquisition_finished = true;
    data.status_change.notify_all();
    status_lock.unlock();
    t.join();
}

void Acquisition::print_summary() const {
    std::cerr << "Hops: "
             << params.hops<< std::endl;
     std::cerr << "Gain of the device: "
              << params.gain<< std::endl;
    std::cerr << "Length of FFT: "
              << (int64_t)params.N<< std::endl;
    std::cerr << "Actual number of (complex) samples collected: "
              << (int64_t)params.N * data.repeats_done << std::endl;
    std::cerr << "Actual number of device readouts: " << deviceReadouts << std::endl;
    std::cerr << "Number of successful readouts: " << successfulReadouts << std::endl;
    std::cerr << "Actual number of averaged spectra: " << data.repeats_done << std::endl;
    std::cerr << "Effective integration time: " <<
        (double)params.N * data.repeats_done / actual_samplerate << " seconds" << std::endl;
}

void Acquisition::write_data() const {

    std::ofstream binfile;
    double pwrdb = 0.0;
    float fpwrdb = 0.0;
    double freq = 0.0;
    int initialBIN, finalBIN, baselineOffset;

    //Interpolate the central point, to cancel DC bias.
    data.pwr[params.N/2] = (data.pwr[params.N/2 - 1] + data.pwr[params.N/2+1]) / 2;

    // Calculate the precision needed for displaying the frequency.
    const int extraDigitsFreq = 2;
    const int significantPlacesFreq =
        ceil(floor(log10(tuned_freq)) - log10(actual_samplerate/params.N) + 1 + extraDigitsFreq);
    const int significantPlacesPwr = 6;

    /*
    the crop percentage will be used to:
      - in this function: to exclude from output half of (cropPercentage * bins) on each side of the FFT
      - in function Plan(): only if hops are performed: calculate a frequency offset by which the center frequency
        of each hop scan should be lowered in order to produce a continuous and not overlapping
        series of bins for the overall frequency range
      - calculate the index offset when subtracting the baseline values
  */
    if( (params.freq_hopping_isSet) && (params.cropPercentage > 0) )
    {
        excludedBINS = int( params.cropPercentage * params.N / 100 ) / 2;
        initialBIN = excludedBINS;
        finalBIN = params.N - excludedBINS;

        actualBINS = (params.N - (excludedBINS * 2)) * hops;
        baselineOffset = (params.N - (excludedBINS * 2)) * (currentHopNumber - 1);
    }
    else
    {
        excludedBINS = 0;
        initialBIN = 0;
        finalBIN = params.N ;

        actualBINS = params.N;
        // 0 offset for single hop scans
        baselineOffset = 0;
    }

    //for (int i = 0; i < params.N; i++) {
    for (int i = initialBIN; i < finalBIN; i++) {

        freq = tuned_freq + (i - params.N/2.0) * actual_samplerate / params.N;

        if( params.linear ) {
            pwrdb = (data.pwr[i] / data.repeats_done / params.N / actual_samplerate)
            - (params.baseline ? aux.baseline_values[baselineOffset+i-excludedBINS] : 0);
        }
        else {
            pwrdb = (10*log10(data.pwr[i] / data.repeats_done / params.N / actual_samplerate))
            - (params.baseline ? aux.baseline_values[baselineOffset+i-excludedBINS] : 0);
        }

        if( params.matrixMode ) {
            // we are accumulating a double, so 8 bytes, removed the sizeof()
            // we are writing a float, so 4 bytes
            // binary file size for 15 mins with N=100 now 43.4 MB instead of 86.8 MB )
            fpwrdb=pwrdb;
            binfile.write( (char*)&fpwrdb, 4 );
        }
        else
        {
            std::cout << std::setprecision(significantPlacesFreq)
            << freq
            << " "
            << std::setprecision(significantPlacesPwr)
            << pwrdb
            << std::endl;
        }
    }




}
float* Acquisition::caculatePwr() {
    float pwrdb = 0.0;
    int initialBIN, finalBIN, baselineOffset;

    // Cấp phát bộ nhớ động cho mảng float
    float* fftMagnitude = new float[params.N];
    // Interpolate the central point, to cancel DC bias.
    data.pwr[params.N / 2] = (data.pwr[params.N / 2 - 1] + data.pwr[params.N / 2 + 1]) / 2;

    if ((params.freq_hopping_isSet) && (params.cropPercentage > 0)) {
        excludedBINS = int(params.cropPercentage * params.N / 100) / 2;
        initialBIN = excludedBINS;
        finalBIN = params.N - excludedBINS;

        actualBINS = (params.N - (excludedBINS * 2)) * hops;
        baselineOffset = (params.N - (excludedBINS * 2)) * (currentHopNumber - 1);
    } else {
        excludedBINS = 0;
        initialBIN = 0;
        finalBIN = params.N;

        actualBINS = params.N;
        // 0 offset for single hop scans
        baselineOffset = 0;
    }

    for (int i = initialBIN; i < finalBIN; i++) {
        //freq = tuned_freq + (i - params.N / 2.0) * actual_samplerate / params.N;

        if (params.linear) {
            pwrdb = (data.pwr[i] / data.repeats_done / params.N / actual_samplerate)
                - (params.baseline ? aux.baseline_values[baselineOffset + i - excludedBINS] : 0);
        } else {
            pwrdb = (10 * log10(data.pwr[i] / data.repeats_done / params.N / actual_samplerate))
                - (params.baseline ? aux.baseline_values[baselineOffset + i - excludedBINS] : 0);
        }
        fftMagnitude[i] = pwrdb; // Gán giá trị vào mảng
    }

    return fftMagnitude; // Trả về con trỏ mảng
}
// Get current date/time, format is "YYYY-MM-DD HH:mm:ss UTC"
std::string Acquisition::currentDateTime() {
    time_t now = std::time(0);
    char buf[80];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %X UTC", std::gmtime(&now));
    return buf;
}
