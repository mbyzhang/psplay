#ifndef __FTIMER_H__
#define __FTIMER_H__

#include <sys/time.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct {

#ifdef __linux__
    int timerfd;
#else
    struct timespec last_tick;
    struct timespec interval;
#endif
    int flags;
    void (*cb)(void*);
    void* cb_args;
    bool paused;
    pthread_mutex_t paused_mutex;
    pthread_cond_t paused_cond;
    bool pausing;
    pthread_mutex_t control_mutex;
    pthread_cond_t control_cond;
    bool exiting;
    pthread_t worker_thread;
} ftimer_t;

#define FTIMER_START_PAUSED         1
#define FTIMER_COMPENSATE_MISSES    2
#define FTIMER_RUN_ASYNC            4
#define FTIMER_RUN_RT               8

void ftimer_create(ftimer_t* ftimer, int flags, struct timespec interval, void (*cb)(void*), void* args);
void ftimer_run(ftimer_t* ftimer);
void ftimer_pause(ftimer_t* ftimer, bool wait_for_paused);
void ftimer_unpause(ftimer_t* ftimer);
void ftimer_exit(ftimer_t* ftimer);
void ftimer_destroy(ftimer_t* ftimer);

#endif
