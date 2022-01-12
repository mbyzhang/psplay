#ifndef __FSK_H__
#define __FSK_H__

#include <sys/time.h>
#include <stddef.h>

#include "simple_tone_gen.h"

typedef struct {
    simple_tone_gen_t* tone_gen;
    double f1;
    double f2;
    struct timeval symbol_duration;
} fsk_t;

int fsk_init(fsk_t* fsk, simple_tone_gen_t* tone_gen, double f1, double f2, struct timeval symbol_duration);
void fsk_start(fsk_t* fsk);
void fsk_send_symbol(fsk_t* fsk, int symbol);
void fsk_send_sequence(fsk_t* fsk, uint8_t* data, size_t n_bits);
void fsk_destroy(fsk_t* fsk);

#endif
