#ifndef __DBPSK_H__
#define __DBPSK_H__

#include <sys/time.h>

#include "bitstream.h"
#include "ftimer.h"
#include "multi_tone_gen.h"

typedef struct {
    multi_tone_gen_t tone_gen;
    ftimer_t symbol_timer;
    double carrier_freq;
    struct timespec symbol_duration;
    unsigned int last_symbol;
    void (*cb)(int, void*);
    void* cb_args;
} dbpsk_t;

void dbpsk_init(dbpsk_t* dbpsk, double carrier_freq, struct timespec symbol_duration, void (*cb)(int, void *), void *cb_args);
void dbpsk_start(dbpsk_t* dbpsk);
void dbpsk_send_symbol(dbpsk_t* dbpsk, unsigned int symbol);
void dbpsk_stop(dbpsk_t* dbpsk);
ssize_t dbpsk_send_sequence(dbpsk_t* dbpsk, bitstream_t* s);
void dbpsk_destroy(dbpsk_t* dbpsk);

#endif