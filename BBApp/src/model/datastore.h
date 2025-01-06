

#ifndef DATASTORE_H
#define DATASTORE_H

#include <complex>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <vector>
#include "../lib/fftw3.h"

#include "../lib/params.h"

class Datastore {
public:
    const Params& params;
    int64_t repeats_done ;

    std::mutex status_mutex;
    // Access to the following objects must be protected by locking
    // status_mutex.
    std::deque<std::vector<uint8_t>*> empty_buffers;
    std::deque<std::vector<uint8_t>*> occupied_buffers;
    bool acquisition_finished;
    std::condition_variable status_change;
    std::vector<int> queue_histogram;

    std::vector<float>& window_values;

    std::complex<float> *inbuf, *outbuf;
    fftwf_plan plan;
    std::vector<double> pwr;

    Datastore(const Params& params, std::vector<float>& window_values);
    ~Datastore();

    // Delete these so we don't accidentally mess anything up by copying
    // pointers to fftw_malloc'd buffers.

    // This function will be started in a separate thread.
    void fftThread();
    void printQueueHistogram() const;
};

#endif // DATASTORE_H
