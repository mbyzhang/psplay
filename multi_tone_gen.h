#ifndef __MULTI_TONE_GEN_H__
#define __MULTI_TONE_GEN_H__

#include <sys/time.h>
#include <pthread.h>

#include "cpu_spinner.h"
#include "ftimer.h"


#define MULTI_TONE_GEN_MAX_TONES 64

typedef struct {
    const double* freqs;
    int n_freqs;
    ftimer_t timers[MULTI_TONE_GEN_MAX_TONES];
    int status;
    pthread_mutex_t status_mutex;
    int freq_idx;
    void (*cb)(int, void*);
    void* cb_args;
} multi_tone_gen_t;

void multi_tone_gen_init(multi_tone_gen_t* tone_gen, const double* freqs, int n_freqs, int ext_timer_flags, void (*cb)(int, void*), void* cb_args);
void multi_tone_gen_switch_frequency(multi_tone_gen_t* tone_gen, int freq_idx);
void multi_tone_gen_switch_phase(multi_tone_gen_t* tone_gen);
void multi_tone_gen_destroy(multi_tone_gen_t* tone_gen);

#endif
