
#ifndef PARAMS_H
#define PARAMS_H

#include <string>
#include "exceptions.h"

const int base_buf = 16384;
const int default_buf_multiplier = 1;

class Params {
public:
    Params();
    int N ;
    int dev_index;
    int gain;
    int cfreq ;
    int hops;
    int startfreq;
    int stopfreq ;
    int sample_rate;
    double integration_time;
    bool integration_time_isSet ;
    int buffers;
    int buf_length;
    bool buf_length_isSet;
    double min_overlap ;
    double cropPercentage ;
    int ppm_error ;
    bool endless;
    bool strict_time;
    bool baseline ;
    std::string baseline_file;
    bool window;
    std::string window_file;
    bool freq_hopping_isSet;
    //It is senseless to waste a full buffer of data unless instructed to do so.
    int repeats ;
    int outcnt ;
    double session_duration ;
    bool session_duration_isSet ;
    bool linear;
    bool talkless ;  // default: verbose
    bool matrixMode;  // default: original rtl-power-fftw output format
    int finalfreq;
    std::string matrix_file;  // just name without extension
    std::string bin_file; // name with .bin extension
    std::string freq_file; // name with .frq extension
    std::string meta_file; // name with .met extension
};

#endif // PARAMS_H
