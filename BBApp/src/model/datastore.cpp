
#include <iostream>
#include "datastore.h"
#include <QTime>
#include <QDebug>
Datastore::Datastore(const Params& params_, std::vector<float>& window_values_) :
    params(params_), queue_histogram(params.buffers+1, 0),
    window_values(window_values_), pwr(params.N)
{
    repeats_done = 0;
    acquisition_finished = false;
    for (int i = 0; i < params.buffers; i++)
        empty_buffers.push_back(new std::vector<uint8_t>(params.buf_length));

    inbuf = (std::complex<float>*)fftwf_alloc_complex(params.N);
    outbuf = (std::complex<float>*)fftwf_alloc_complex(params.N);
    plan = fftwf_plan_dft_1d(params.N, (fftwf_complex*)inbuf, (fftwf_complex*)outbuf,
                             FFTW_FORWARD, FFTW_MEASURE);
}

Datastore::~Datastore() {
    for (auto& buffer : empty_buffers)
        delete buffer;

    for (auto& buffer : occupied_buffers)
        delete buffer;

    fftwf_destroy_plan(plan);
    fftwf_free(inbuf);
    fftwf_free(outbuf);
}

void Datastore::fftThread()
{

    std::unique_lock<std::mutex>
        status_lock(status_mutex, std::defer_lock);
    int fft_pointer = 0;
    while (true) {
        status_lock.lock();
        while (occupied_buffers.empty() && !acquisition_finished)
            status_change.wait(status_lock);
        if (occupied_buffers.empty()) {
            break;
        }
        std::vector<uint8_t>& buffer(*occupied_buffers.front());
        occupied_buffers.pop_front();
        status_lock.unlock();
        //A neat new loop to avoid having to have data buffer aligned with fft buffer.
        unsigned int buffer_pointer = 0;
        while (buffer_pointer < buffer.size() && repeats_done < params.repeats ) {
            while (fft_pointer < params.N && buffer_pointer < buffer.size()) {
                //The magic aligment happens here: we have to change the phase of each next complex sample
                //by pi - this means that even numbered samples stay the same while odd numbered samples
                //get multiplied by -1 (thus rotated by pi in complex plane).
                //This gets us output spectrum shifted by half its size - just what we need to get the output right.
                const float multiplier = (fft_pointer % 2 == 0 ? 1 : -1);
                std::complex<float> bfr_val(buffer[buffer_pointer], buffer[buffer_pointer+1]);
                inbuf[fft_pointer] = (bfr_val - std::complex<float>(127.0, 127.0)) * multiplier;
                if (params.window)
                    inbuf[fft_pointer] *= window_values[fft_pointer];
                buffer_pointer += 2;
                fft_pointer++;
            }
            if (fft_pointer == params.N) {
                fftwf_execute(plan);
                for (int i = 0; i < params.N; i++) {
                    pwr[i] += pow(outbuf[i].real(), 2) + pow(outbuf[i].imag(), 2);
                }
                repeats_done++;
                fft_pointer = 0;
            }
        }

        status_lock.lock();
        empty_buffers.push_back(&buffer);
        status_change.notify_all();
        status_lock.unlock();
    }

}

void Datastore::printQueueHistogram() const {
    std::cerr << "Buffer queue histogram: ";
    for (auto size : queue_histogram)
        std::cerr << size << " ";
    std::cerr << std::endl;
}
