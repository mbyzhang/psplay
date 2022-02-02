#include "fsk.h"
#include <stdbool.h>
#define BUF_SIZE 1024

int fsk_init(fsk_t* fsk, simple_tone_gen_t* tone_gen, double f1, double f2, struct timeval symbol_duration) {
    fsk->f1 = f1;
    fsk->f2 = f2;
    fsk->tone_gen = tone_gen;
    fsk->symbol_duration = symbol_duration;
    return 0;
}

void fsk_start(fsk_t* fsk) {
    simple_tone_gen_start(fsk->tone_gen, fsk->symbol_duration);
}

void fsk_send_symbol(fsk_t* fsk, int symbol) {
    double freq = symbol ? fsk->f2 : fsk->f1;
    simple_tone_gen_step(fsk->tone_gen, freq);
}

ssize_t fsk_send_sequence(fsk_t* fsk, bitstream_t* s) {
    fsk_start(fsk);

    uint8_t buf[BUF_SIZE];
    size_t tx_total_bits = 0;

    while (true) {
        size_t n_bits = bitstream_read(s, buf, sizeof(buf) * 8);
        if (n_bits == 0) break;
        if (n_bits < 0) return n_bits;
        tx_total_bits += n_bits;

        size_t p = 0;
        for (int i = 0; i < n_bits; i += 8) {
            uint8_t x = buf[p++];
            for (int j = i; j < n_bits && j < i + 8; j++) {
                fsk_send_symbol(fsk, x & (1 << (j - i)));
            }
        }
    }
    return tx_total_bits;
}

void fsk_destroy(fsk_t* fsk) {
    return;
}
