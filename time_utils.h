#ifndef __TIME_UTILS_H__
#define __TIME_UTILS_H__

#include <time.h>
#include <sys/time.h>
#include <stdint.h>

#define GIGA 1000000000L

void timespec_diff(struct timespec* a, struct timespec b) {
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

void timespec_step(struct timespec* a, struct timespec b) {
    a->tv_sec += b.tv_sec;
    a->tv_nsec += b.tv_nsec;
    a->tv_sec += a->tv_nsec / GIGA;
    a->tv_nsec %= GIGA;
}

struct timeval us_to_timeval(uint64_t us) {
    return (struct timeval){ us / 1000000L, us % 1000000UL };
}

struct timeval timespec_to_timeval(struct timespec a) {
    return (struct timeval){ a.tv_sec, a.tv_nsec / 1000L };
}

struct timespec timeval_to_timespec(struct timeval a) {
    return (struct timespec){ a.tv_sec, a.tv_usec * 1000L };
}

#endif