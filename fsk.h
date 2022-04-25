#ifndef __FSK_H__
#define __FSK_H__

#include <sys/time.h>
#include <sys/types.h>
#include <stddef.h>

#include "multi_tone_gen.h"
#include "bitstream.h"
#include "ftimer.h"

typedef struct {
    multi_tone_gen_t tone_gen;
    ftimer_t symbol_timer;
    const double* freqs;
    int m_exp;
    struct timespec symbol_duration;
    void (*cb)(int, void*);
    void* cb_args;
} fsk_t;

int fsk_init(fsk_t* fsk, const double* freqs, int m_exp, struct timespec symbol_duration, void (*cb)(int, void *), void *cb_args);
void fsk_start(fsk_t* fsk);
void fsk_send_symbol(fsk_t* fsk, unsigned int symbol);
void fsk_stop(fsk_t* fsk);
ssize_t fsk_send_sequence(fsk_t* fsk, bitstream_t* bitstream);
void fsk_destroy(fsk_t* fsk);

#endif
