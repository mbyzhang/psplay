#ifndef __UTILS_H__
#define __UTILS_H__

#include "simple_tone_gen.h"
#include <time.h>
#include <stdint.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#define MEGA 1000000L
#define GIGA 1000000000L

#define BIT_ONES(n) ((1U << (n)) - 1U)
#define MIN(x, y) ((x) < (y) ? (x): (y))
#define IS_POT(x) ((x != 0) && ((x & (x - 1)) == 0))
#define CHECK_ERROR(expr) if ((ret = (expr)) < 0) goto fail
#define CHECK_ERROR_NZ(expr) if ((ret = (expr)) != 0) goto fail
#define ASSERT_SUCCESS_Z(expr) if ((expr) == 0) abort();
#define ASSERT_SUCCESS_NZ(expr) if ((expr) != 0) abort();

static inline void thread_set_priority_to_max() {
    static bool warning_shown = false;
    const int policy = SCHED_RR;
    struct sched_param param = {
        .sched_priority = sched_get_priority_max(policy)
    };
    int ret = pthread_setschedparam(pthread_self(), policy, &param);
    if (ret != 0 && !warning_shown) {
        perror("Failed to set thread priority, performance may be affected");
        warning_shown = true;
    }
}

static inline unsigned int log2_int(unsigned int x) {
  unsigned int y = 0;
  while (x >>= 1) y++;
  return y;
}

static inline void play_chirp(simple_tone_gen_t* tone_gen, double freq_start, double freq_end, double freq_step, struct timeval step_duration) {
    simple_tone_gen_start(tone_gen, step_duration);
    for (double i = freq_start; i <= freq_end; i += freq_step) {
        simple_tone_gen_step(tone_gen, i);
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
    return (struct timeval){ us / MEGA, us % MEGA };
}

static inline struct timespec ns_to_timespec(uint64_t ns) {
    return (struct timespec){ ns / GIGA, ns % GIGA };
}

static inline struct timespec hz_to_period_timespec(double hz) {
    return ns_to_timespec(GIGA / hz);
}

static inline int timespec_cmp(struct timespec a, struct timespec b) {
    if (a.tv_sec > b.tv_sec || (a.tv_sec == b.tv_sec && a.tv_nsec > b.tv_nsec)) {
        return 1;
    }
    else if (a.tv_sec == b.tv_sec && a.tv_nsec == b.tv_nsec) {
        return 0;
    }
    else {
        return -1;
    }
}

static inline struct timeval timespec_to_timeval(struct timespec a) {
    return (struct timeval){ a.tv_sec, a.tv_nsec / 1000L };
}

static inline struct timespec timeval_to_timespec(struct timeval a) {
    return (struct timespec){ a.tv_sec, a.tv_usec * 1000L };
}

#endif
