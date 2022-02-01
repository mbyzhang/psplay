#ifndef __UTILS_H__
#define __UTILS_H__

#include "simple_tone_gen.h"
#include <time.h>
#include <stdint.h>
#include <sys/time.h>

#define GIGA 1000000000L
#define BIT_ONES(n) ((1U << (n)) - 1U)
#define MIN(x, y) ((x) < (y) ? (x): (y))

static inline void play_chirp(simple_tone_gen_t* tone_gen, double freq_start, double freq_end, double freq_step, struct timeval step_duration) {
    for (double i = freq_start; i <= freq_end; i += freq_step) {
        simple_tone_gen_play(tone_gen, i, step_duration);
    }
}

static inline void timespec_diff(struct timespec* a, struct timespec b) {
    a->tv_sec = b.tv_sec - a->tv_sec;
    a->tv_nsec = b.tv_nsec - a->tv_nsec;
    if (a->tv_sec < 0L) {
        a->tv_sec = 0L;
        a->tv_nsec = 0L;
    } else if (a->tv_nsec < 0L) {
        if (a->tv_sec == 0L) {
            a->tv_sec = 0L;
            a->tv_nsec = 0L;
        } else {
            a->tv_sec = a->tv_sec - 1L;
            a->tv_nsec = a->tv_nsec + GIGA;
        }
    } else {}
}

static inline void timespec_step(struct timespec* a, struct timespec b) {
    a->tv_sec += b.tv_sec;
    a->tv_nsec += b.tv_nsec;
    a->tv_sec += a->tv_nsec / GIGA;
    a->tv_nsec %= GIGA;
}

static inline struct timeval us_to_timeval(uint64_t us) {
    return (struct timeval){ us / 1000000L, us % 1000000UL };
}

static inline struct timeval timespec_to_timeval(struct timespec a) {
    return (struct timeval){ a.tv_sec, a.tv_nsec / 1000L };
}

static inline struct timespec timeval_to_timespec(struct timeval a) {
    return (struct timespec){ a.tv_sec, a.tv_usec * 1000L };
}

#endif
