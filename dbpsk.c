#include "dbpsk.h"
#include "utils.h"

#include <stdbool.h>
#include <math.h>

void dbpsk_init(dbpsk_t* dbpsk, double carrier_freq, struct timespec symbol_duration, void (*cb)(int, void *), void *cb_args) {
    dbpsk->carrier_freq = carrier_freq;
    dbpsk->symbol_duration = symbol_duration;
    dbpsk->cb = cb;
    dbpsk->cb_args = cb_args;
    dbpsk->last_symbol = 0;
}

void dbpsk_start(dbpsk_t* dbpsk) {
    multi_tone_gen_init(&dbpsk->tone_gen, &dbpsk->carrier_freq, 1, FTIMER_COMPENSATE_MISSES, dbpsk->cb, dbpsk->cb_args);
    multi_tone_gen_switch_frequency(&dbpsk->tone_gen, 0);
    ftimer_create(&dbpsk->symbol_timer, FTIMER_RUN_RT | FTIMER_RUN_ASYNC | FTIMER_COMPENSATE_MISSES, dbpsk->symbol_duration, NULL, NULL);
}

void dbpsk_send_symbol(dbpsk_t* dbpsk, unsigned int symbol) {
    if (symbol != dbpsk->last_symbol) {
        multi_tone_gen_switch_phase(&dbpsk->tone_gen);
    }

    dbpsk->last_symbol = symbol;
    ftimer_wait(&dbpsk->symbol_timer);
}

void dbpsk_stop(dbpsk_t* dbpsk) {
    ftimer_destroy(&dbpsk->symbol_timer);
    multi_tone_gen_destroy(&dbpsk->tone_gen);
}

ssize_t dbpsk_send_sequence(dbpsk_t* dbpsk, bitstream_t* s) {
    dbpsk_start(dbpsk);

    ssize_t tx_total_bits = 0;
    
    while (true) {
        unsigned int symbol = 0;
        ssize_t n_bits = bitstream_read(s, &symbol, 1);
        if (n_bits == 0) break;
        else if (n_bits < 0) return n_bits;
        tx_total_bits += n_bits;
        dbpsk_send_symbol(dbpsk, symbol);
    }

    dbpsk_stop(dbpsk);

    return tx_total_bits;
}

void dbpsk_destroy(dbpsk_t* dbpsk) {
    return;
}
