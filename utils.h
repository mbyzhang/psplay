#ifndef __UTILS_H__
#define __UTILS_H__

#include "simple_tone_gen.h"
#include <sys/time.h>

void play_chirp(simple_tone_gen_t* tone_gen, double freq_start, double freq_end, double freq_step, struct timeval step_duration) {
    for (double i = freq_start; i <= freq_end; i += freq_step) {
        simple_tone_gen_play(tone_gen, i, step_duration);
    }
}

#endif
