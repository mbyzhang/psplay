#ifndef __SIMPLE_TONE_GEN_H__
#define __SIMPLE_TONE_GEN_H__

#include <sys/time.h>
#include <event2/event.h>

#include "cpu_spinner.h"

typedef struct {
    cpu_spinner_t* spinner;
    struct timespec last_step_time;
    struct timespec step_duration;
    int status;
} simple_tone_gen_t;

int simple_tone_gen_init(simple_tone_gen_t* simple_tone_gen, cpu_spinner_t* spinner);
void simple_tone_gen_play(simple_tone_gen_t* simple_tone_gen, double freq, struct timespec duration_ts);
void simple_tone_gen_start(simple_tone_gen_t* simple_tone_gen, struct timespec step_duration);
void simple_tone_gen_step(simple_tone_gen_t* simple_tone_gen, double freq);
void simple_tone_gen_destroy(simple_tone_gen_t* simple_tone_gen);

#endif
