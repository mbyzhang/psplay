#ifndef __FSK_H__
#define __FSK_H__

#include <sys/time.h>
#include <sys/types.h>
#include <stddef.h>

#include "simple_tone_gen.h"
#include "bitstream.h"

typedef struct {
    simple_tone_gen_t* tone_gen;
    const double* freqs;
    int m_exp;
    struct timespec symbol_duration;
} fsk_t;

int fsk_init(fsk_t* fsk, simple_tone_gen_t* tone_gen, const double* freqs, int m_exp, struct timespec symbol_duration);
void fsk_start(fsk_t* fsk);
void fsk_send_symbol(fsk_t* fsk, unsigned int symbol);
ssize_t fsk_send_sequence(fsk_t* fsk, bitstream_t* bitstream);
void fsk_destroy(fsk_t* fsk);

#endif
