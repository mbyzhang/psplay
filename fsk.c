#include "fsk.h"

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

void fsk_destroy(fsk_t* fsk) {
    return;
}
