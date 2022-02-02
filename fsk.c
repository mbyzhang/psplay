#include "fsk.h"
#include <stdbool.h>

#include "utils.h"

int fsk_init(fsk_t* fsk, simple_tone_gen_t* tone_gen, const double* freqs, int m_exp, struct timeval symbol_duration) {
    fsk->freqs = freqs;
    fsk->m_exp = m_exp;
    fsk->tone_gen = tone_gen;
    fsk->symbol_duration = symbol_duration;
    return 0;
}

void fsk_start(fsk_t* fsk) {
    simple_tone_gen_start(fsk->tone_gen, fsk->symbol_duration);
}

void fsk_send_symbol(fsk_t* fsk, unsigned int symbol) {
    double freq = fsk->freqs[symbol];
    simple_tone_gen_step(fsk->tone_gen, freq);
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

    return tx_total_bits;
}

void fsk_destroy(fsk_t* fsk) {
    return;
}
