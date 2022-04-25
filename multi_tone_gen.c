#include "multi_tone_gen.h"
#include "utils.h"

#include <stdbool.h>

static void toggle(multi_tone_gen_t* tone_gen) {
    pthread_mutex_lock(&tone_gen->status_mutex);
    tone_gen->status = !tone_gen->status;
    tone_gen->cb(tone_gen->status, tone_gen->cb_args);
    // printf("status = %d\n", tone_gen->status);
    pthread_mutex_unlock(&tone_gen->status_mutex);
}

void multi_tone_gen_init(multi_tone_gen_t* tone_gen, const double* freqs, int n_freqs, int ext_timer_flags, void (*cb)(int, void*), void* cb_args) {
    tone_gen->freq_idx = -1;
    tone_gen->freqs = freqs;
    tone_gen->n_freqs = n_freqs;
    tone_gen->cb = cb;
    tone_gen->cb_args = cb_args;
    tone_gen->status = 0;

    CHECK_ERROR_NE0(pthread_mutex_init(&tone_gen->status_mutex, NULL));

    for (int i = 0; i < n_freqs; ++i) {
        ftimer_create(
            &tone_gen->timers[i],
            FTIMER_START_PAUSED | FTIMER_RUN_ASYNC | FTIMER_RUN_RT | ext_timer_flags,
            hz_to_period_timespec(freqs[i] * 2.0),
            (void(*)(void*))toggle,
            tone_gen
        );
    }
}

void multi_tone_gen_switch_frequency(multi_tone_gen_t* tone_gen, int freq_idx) {
    if (freq_idx < -1 || freq_idx >= tone_gen->n_freqs) return;

    if (tone_gen->freq_idx != -1) {
        ftimer_pause(&tone_gen->timers[tone_gen->freq_idx], true);
    }
    tone_gen->freq_idx = freq_idx;
    if (freq_idx != -1) {
        ftimer_unpause(&tone_gen->timers[freq_idx]);
    }
}

void multi_tone_gen_switch_phase(multi_tone_gen_t* tone_gen) {
    toggle(tone_gen);
}

void multi_tone_gen_destroy(multi_tone_gen_t* tone_gen) {
    for (int i = 0; i < tone_gen->n_freqs; ++i) {
        ftimer_destroy(&tone_gen->timers[i]);
    }

    pthread_mutex_destroy(&tone_gen->status_mutex);
}
