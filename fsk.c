#include "fsk.h"
#include <stdbool.h>

#include "utils.h"

int fsk_init(fsk_t* fsk, const double* freqs, int m_exp, struct timespec symbol_duration, void (*cb)(int, void *), void *cb_args) {
    fsk->freqs = freqs;
    fsk->m_exp = m_exp;
    fsk->symbol_duration = symbol_duration;
    fsk->cb = cb;
    fsk->cb_args = cb_args;
    return 0;
}

void fsk_start(fsk_t* fsk) {
    multi_tone_gen_init(&fsk->tone_gen, fsk->freqs, 1 << fsk->m_exp, 0, fsk->cb, fsk->cb_args);
    ftimer_create(&fsk->symbol_timer, FTIMER_RUN_RT | FTIMER_RUN_ASYNC | FTIMER_COMPENSATE_MISSES, fsk->symbol_duration, NULL, NULL);
}

void fsk_send_symbol(fsk_t* fsk, unsigned int symbol) {
    multi_tone_gen_switch_frequency(&fsk->tone_gen, symbol);
    ftimer_wait(&fsk->symbol_timer);
}

void fsk_stop(fsk_t* fsk) {
    ftimer_destroy(&fsk->symbol_timer);
    multi_tone_gen_destroy(&fsk->tone_gen);
}

ssize_t fsk_send_sequence(fsk_t* fsk, bitstream_t* s) {
    fsk_start(fsk);

    size_t tx_total_bits = 0;

    while (true) {
        unsigned int symbol = 0;
        size_t n_bits = bitstream_read(s, &symbol, fsk->m_exp);
        if (n_bits == 0) break;
        if (n_bits < 0) return n_bits;
        tx_total_bits += n_bits;
        fsk_send_symbol(fsk, symbol);
    }

    fsk_stop(fsk);

    return tx_total_bits;
}

void fsk_destroy(fsk_t* fsk) {
    return;
}
