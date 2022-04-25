#include "ftimer.h"
#include "utils.h"

#ifdef __linux__
#include <sys/timerfd.h>
#endif

#include <sys/time.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

void ftimer_create(ftimer_t *ftimer, int flags, struct timespec interval, void (*cb)(void *), void *args)
{
    ftimer->flags = flags;
    ftimer->cb = cb;
    ftimer->cb_args = args;
    ftimer->pausing = flags & FTIMER_START_PAUSED;
    ftimer->paused = false;
    ftimer->exiting = false;

    CHECK_ERROR_NE0(pthread_mutex_init(&ftimer->control_mutex, NULL));
    CHECK_ERROR_NE0(pthread_cond_init(&ftimer->control_cond, NULL));
    CHECK_ERROR_NE0(pthread_mutex_init(&ftimer->paused_mutex, NULL));
    CHECK_ERROR_NE0(pthread_cond_init(&ftimer->paused_cond, NULL));
    CHECK_ERROR_NE0(sem_init(&ftimer->sem, 0, 0));

#ifdef __linux__
    struct itimerspec value = {
        .it_interval = interval,
        .it_value = interval,
    };

    CHECK_ERROR_EQN1(ftimer->timerfd = timerfd_create(CLOCK_MONOTONIC, 0));
    CHECK_ERROR_EQN1(timerfd_settime(ftimer->timerfd, 0, &value, NULL));
#else
    ftimer->interval = interval;
    clock_gettime(CLOCK_SOURCE, &ftimer->last_tick);
#endif

    if (flags & FTIMER_RUN_ASYNC)
    {
        pthread_create(&ftimer->worker_thread, NULL, (void *)(void *)ftimer_run, ftimer);
    }
}

void ftimer_run(ftimer_t *ftimer)
{
    if (ftimer->flags & FTIMER_RUN_RT) {
        thread_set_priority_to_max();
    }

    uint64_t exp = 0;
    while (true)
    {
        pthread_mutex_lock(&ftimer->control_mutex);
        if (ftimer->pausing)
        {
            pthread_mutex_lock(&ftimer->paused_mutex);
            ftimer->paused = true;
            pthread_cond_broadcast(&ftimer->paused_cond);
            pthread_mutex_unlock(&ftimer->paused_mutex);

            while (ftimer->pausing)
            {
                pthread_cond_wait(&ftimer->control_cond, &ftimer->control_mutex);
            }
            pthread_mutex_lock(&ftimer->paused_mutex);
            ftimer->paused = false;
            pthread_mutex_unlock(&ftimer->paused_mutex);
        }

        if (ftimer->exiting)
        {
            return;
        }
        pthread_mutex_unlock(&ftimer->control_mutex);

#ifdef __linux__
        ssize_t s = read(ftimer->timerfd, &exp, sizeof(uint64_t));
        CHECK_ERROR_NE0(s - sizeof(uint64_t));

        for (uint64_t i = 0; i < exp; ++i)
        {
            if (ftimer->cb != NULL) ftimer->cb(ftimer->cb_args);
            CHECK_ERROR_NE0(sem_post(&ftimer->sem));
            if (ftimer->flags & FTIMER_COMPENSATE_MISSES) break;
        }
#else
        timespec_step(&ftimer->last_tick, ftimer->interval);
        struct timespec now;
        clock_gettime(CLOCK_SOURCE, &now);
        timespec_diff(&now, ftimer->last_tick);

        if (!(ftimer->flags & FTIMER_COMPENSATE_MISSES))
        {
            while (timespec_cmp(now, (struct timespec){0, 0}) <= 0) {
                timespec_step(&ftimer->last_tick, ftimer->interval);
                timespec_diff(&now, ftimer->last_tick);
            }
        }

        nanosleep(&now, NULL);
        if (ftimer->cb != NULL) ftimer->cb(ftimer->cb_args);
        CHECK_ERROR_NE0(sem_post(&ftimer->sem));
#endif
    }
}

void ftimer_pause(ftimer_t *ftimer, bool wait_for_paused)
{
    pthread_mutex_lock(&ftimer->control_mutex);
    ftimer->pausing = true;
    pthread_mutex_unlock(&ftimer->control_mutex);

    if (!wait_for_paused)
        return;

    pthread_mutex_lock(&ftimer->paused_mutex);
    while (!ftimer->paused)
    {
        pthread_cond_wait(&ftimer->paused_cond, &ftimer->paused_mutex);
    }
    pthread_mutex_unlock(&ftimer->paused_mutex);
}

void ftimer_unpause(ftimer_t *ftimer)
{
    pthread_mutex_lock(&ftimer->control_mutex);
    ftimer->pausing = false;
    pthread_cond_broadcast(&ftimer->control_cond);
    pthread_mutex_unlock(&ftimer->control_mutex);
}

void ftimer_wait(ftimer_t* ftimer) {
    sem_wait(&ftimer->sem);
}

void ftimer_exit(ftimer_t *ftimer)
{
    pthread_mutex_lock(&ftimer->control_mutex);
    ftimer->exiting = true;
    pthread_mutex_unlock(&ftimer->control_mutex);
}

void ftimer_destroy(ftimer_t *ftimer)
{
    ftimer_exit(ftimer);

    if (ftimer->flags & FTIMER_RUN_ASYNC)
    {
        pthread_join(ftimer->worker_thread, NULL);
    }

#ifdef __linux__
    close(ftimer->timerfd);
#endif
    sem_destroy(&ftimer->sem);
    pthread_cond_destroy(&ftimer->paused_cond);
    pthread_mutex_destroy(&ftimer->paused_mutex);
    pthread_cond_destroy(&ftimer->control_cond);
    pthread_mutex_destroy(&ftimer->control_mutex);
}
