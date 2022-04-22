#include "simple_tone_gen.h"
#include <math.h>
#include <stdio.h>
#include <pthread.h>
#include "utils.h"

#ifndef CLOCK_SOURCE
#ifdef __CYGWIN__
#define CLOCK_SOURCE CLOCK_MONOTONIC
#else
#define CLOCK_SOURCE CLOCK_MONOTONIC_RAW
#endif
#endif

int simple_tone_gen_init(simple_tone_gen_t* simple_tone_gen, cpu_spinner_t* spinner) {
    simple_tone_gen->spinner = spinner;
    simple_tone_gen->event_base = event_base_new(); // TODO: error checking

    thread_set_priority_to_max();

    return 0;
}

typedef struct {
    struct event* toggler_ev;
    int status;
    simple_tone_gen_t* simple_tone_gen;
} cb_func_args;

void toggler_cb_func(evutil_socket_t fd, short what, void *arg_raw) {
    cb_func_args* args = (cb_func_args*)arg_raw;
    args->status = !args->status;
    cpu_spinner_spin(args->simple_tone_gen->spinner, (args->status)? CPU_SPINNER_ALL_CORES_ACTIVE : CPU_SPINNER_ALL_CORES_IDLE);
    // printf("toggle status = %d\n", args->status);
}

void full_duration_elasped_cb_func(evutil_socket_t fd, short what, void *arg_raw) {
    cb_func_args* args = (cb_func_args*)arg_raw;
    event_del(args->toggler_ev);
}

struct timeval to_timeval(uint64_t us) {
    struct timeval ret = { us / 1000000ULL, us % 1000000ULL };
    return ret;
}

void simple_tone_gen_play(simple_tone_gen_t* simple_tone_gen, double freq, struct timeval duration) {
    // TODO: error checking
    cb_func_args toggler_args;
    struct timeval half_period;
    struct event* duration_ev;

    duration_ev = event_new(simple_tone_gen->event_base, -1, 0, full_duration_elasped_cb_func, &toggler_args);

    half_period = to_timeval((uint64_t)round(0.5 / freq * 1e6));
    toggler_args.simple_tone_gen = simple_tone_gen;
    toggler_args.status = 0;
    toggler_args.toggler_ev = event_new(simple_tone_gen->event_base, -1, EV_PERSIST, toggler_cb_func, &toggler_args);

    event_add(toggler_args.toggler_ev, &half_period);
    event_add(duration_ev, &duration);

    event_base_dispatch(simple_tone_gen->event_base);

    event_free(duration_ev);
    event_free(toggler_args.toggler_ev);
}

void simple_tone_gen_start(simple_tone_gen_t* simple_tone_gen, struct timeval step_duration) {
    clock_gettime(CLOCK_SOURCE, &simple_tone_gen->last_step_time);
    simple_tone_gen->step_duration = timeval_to_timespec(step_duration);
}

void simple_tone_gen_step(simple_tone_gen_t* simple_tone_gen, double freq) {
    timespec_step(&simple_tone_gen->last_step_time, simple_tone_gen->step_duration);

    struct timespec now;
    clock_gettime(CLOCK_SOURCE, &now);
    timespec_diff(&now, simple_tone_gen->last_step_time);
    simple_tone_gen_play(simple_tone_gen, freq, timespec_to_timeval(now));
}

void simple_tone_gen_destroy(simple_tone_gen_t* simple_tone_gen) {
    event_base_free(simple_tone_gen->event_base);
}
