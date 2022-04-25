#include "simple_tone_gen.h"
#include <math.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "utils.h"
#include "ftimer.h"

int simple_tone_gen_init(simple_tone_gen_t* simple_tone_gen, cpu_spinner_t* spinner) {
    simple_tone_gen->spinner = spinner;
    return 0;
}

static void toggler_cb_func(simple_tone_gen_t* tone_gen) {
    tone_gen->status = !tone_gen->status;
    cpu_spinner_spin(tone_gen->spinner, (tone_gen->status)? CPU_SPINNER_ALL_CORES_ACTIVE : CPU_SPINNER_ALL_CORES_IDLE);
    // printf("toggle status = %d\n", tone_gen->status);
}

void simple_tone_gen_play(simple_tone_gen_t* simple_tone_gen, double freq, struct timespec duration_ts) {
    ftimer_t ftimer;
    ftimer_create(&ftimer, FTIMER_RUN_ASYNC | FTIMER_RUN_RT, hz_to_period_timespec(freq * 2.0), (void(*)(void*))toggler_cb_func, simple_tone_gen);
    nanosleep(&duration_ts, NULL);
    ftimer_destroy(&ftimer);
}

void simple_tone_gen_start(simple_tone_gen_t* simple_tone_gen, struct timespec step_duration) {
    clock_gettime(CLOCK_SOURCE, &simple_tone_gen->last_step_time);
    simple_tone_gen->step_duration = step_duration;
}

void simple_tone_gen_step(simple_tone_gen_t* simple_tone_gen, double freq) {
    timespec_step(&simple_tone_gen->last_step_time, simple_tone_gen->step_duration);

    struct timespec now;
    clock_gettime(CLOCK_SOURCE, &now);
    timespec_diff(&now, simple_tone_gen->last_step_time);
    simple_tone_gen_play(simple_tone_gen, freq, now);
}

void simple_tone_gen_destroy(simple_tone_gen_t* simple_tone_gen) {
}
