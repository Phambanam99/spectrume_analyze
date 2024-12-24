/*
* rtl_power_fftw, program for calculating power spectrum from rtl-sdr reciever.
* Copyright (C) 2015 Klemen Blokar <klemen.blokar@ad-vega.si>
*                    Andrej Lajovic <andrej.lajovic@ad-vega.si>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <list>
#include <math.h>
#include <string>

#include "params.h"
#include "exceptions.h"


Params::Params() {    int N = 512;
     dev_index = 0;
     gain = 372;
     cfreq = 1420405752;
     startfreq = 0;
     stopfreq = 0;
     hops = 1;
     sample_rate = 2000000;
     integration_time = 0;
     integration_time_isSet = false;
     buffers = 5;
     buf_length = base_buf * default_buf_multiplier;
     buf_length_isSet = false;
     min_overlap = 0;
     cropPercentage = 0;
     ppm_error = 0;
     endless = false;
     strict_time = false;
     baseline = false;
     window = false;
     freq_hopping_isSet = false;
    //It is senseless to waste a full buffer of data unless instructed to do so.
     repeats = buf_length/(2*N);
     outcnt = 0;
     session_duration = 0;
     session_duration_isSet = false;
     linear = false;
     talkless = false;  // default: verbose
     matrixMode = false;  // default: original rtl-power-fftw output format
     finalfreq = 0;
}
