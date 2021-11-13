#ifndef __FSK_H__
#define __FSK_H__

#include <sys/time.h>
#include <event2/event.h>

#include "cpu_spinner.h"

typedef struct {
    cpu_spinner_t* spinner;
    struct event_base* event_base;
} fsk_t;

int fsk_init(fsk_t* fsk, cpu_spinner_t* spinner);
void fsk_play_tone(fsk_t* fsk, double freq, struct timeval duration);
void fsk_destroy(fsk_t* fsk);

#endif
