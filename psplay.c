#include <stdio.h>
#include <sched.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sndfile.h>

#include "cpu_spinner.h"
#include "fsk.h"
#include "simple_tone_gen.h"
#include "framer.h"
#include "utils.h"
#include "ping_pong_buf.h"
#include "bitbang_player.h"

#define MODE_ALTERNATING_SYMBOLS 0
#define MODE_MESSAGE 1
#define MODE_CHIRP 2
#define MODE_AUDIOFILE 3
#define BUF_SIZE 1024
#define MAX_FREQ_SIZE 128

#define PARSE_CLI_DOUBLE(var) do { \
    char *endptr;                  \
    var = strtod(optarg, &endptr); \
    if (optarg == endptr) {        \
        fprintf(stderr, "Could not parse command-line argument -%c=%s\n", opt, optarg); \
        exit(EXIT_FAILURE);        \
    }                              \
} while (0);

int main(int argc, char* argv[]) {
    cpu_spinner_t spinner;
    fsk_t fsk;
    simple_tone_gen_t tone_gen;
    framer_t framer;
    ppbuf_t ppbuf;
    bitbang_player_t bbplayer;

    cpu_spinner_init(&spinner, 0);
    simple_tone_gen_init(&tone_gen, &spinner);
    framer_init(&framer, 0.2);

    double freqs[MAX_FREQ_SIZE];
    size_t n_freqs = 0;

    int mode = MODE_ALTERNATING_SYMBOLS;
    uint8_t buf[BUF_SIZE];
    int ret;
    bitstream_t bitstream;

    bool loop = false;
    double loop_delay = 0.5;

    double baudrate = 100.0;

    SNDFILE* sndfile;
    SF_INFO sndfile_info = {
        .format = 0
    };
    const int block_size = 4096;
    double audio_gain = 1.0;

    int opt;
    while ((opt = getopt(argc, argv, ":f:m:lacb:d:i:g:")) != -1) {
        switch (opt) {
        case 'f': { // frequencies
            char* pch;
            pch = strtok(optarg, ",");
            while (pch != NULL && n_freqs < MAX_FREQ_SIZE) {
                char* endptr;
                freqs[n_freqs++] = strtod(pch, &endptr);
                if (pch == endptr) {
                    fprintf(stderr, "Invalid frequency: %s\n", pch);
                    exit(EXIT_FAILURE);
                }
                pch = strtok(NULL, ",");
            }
            if (!IS_POT(n_freqs) || n_freqs < 2) {
                fprintf(stderr, "Number of frequencies must be at least two and a power of two\n");
                exit(EXIT_FAILURE);
            }
            break;
        }
        case 'm': // message
            mode = MODE_MESSAGE;
            size_t msg_len = strlen(optarg);
            int ret = bitstream_init(&bitstream, FRAME_MAX_LENGTH_BITS);
            if (ret < 0) {
                fprintf(stderr, "Could not allocate memory for bitstream\n");
                exit(EXIT_FAILURE);
            }
            ret = framer_frame(&framer, (uint8_t*)optarg, msg_len, &bitstream);
            if (ret < 0) {
                fprintf(stderr, "Could not encode message: %s\n", strerror(-ret));
                exit(EXIT_FAILURE);
            }
            bitstream_dump(&bitstream);
            break;
        case 'l': // loop
            loop = true;
            break;
        case 'a': // alternating symbols
            mode = MODE_ALTERNATING_SYMBOLS;
            break;
        case 'i': // audio file
            mode = MODE_AUDIOFILE;
            sndfile = sf_open(optarg, SFM_READ, &sndfile_info);

            if (sf_error(sndfile)) {
                fprintf(stderr, "Could not open audio file: %s\n", sf_strerror(sndfile));
                exit(EXIT_FAILURE);
            }

            if (sndfile_info.channels != 1) {
                fprintf(stderr, "Could not open audio file: only mono audio is supported\n");
                exit(EXIT_FAILURE);
            }

            break;
        case 'g': // audio gain
            PARSE_CLI_DOUBLE(audio_gain);
            break;
        case 'b': // baudrate
            PARSE_CLI_DOUBLE(baudrate);
            break;
        case 'd': // loop delay
            PARSE_CLI_DOUBLE(loop_delay);
            break;
        case 'c': // chrip
            mode = MODE_CHIRP;
            break;
        default:
            fprintf(stderr, "usage: %s [-acl] [-m message] [-i audio_file] [-f f0,f1,...] [-b baudrate] [-d loop_delay] [-i audio_gain]\n", argv[0]);
            exit(EXIT_FAILURE);
            break;
        }
    }

    if (n_freqs == 0) {
        n_freqs = 2;
        freqs[0] = 3200.0;
        freqs[1] = 3000.0;
    }

    fsk_init(&fsk, &tone_gen, freqs, log2_int(n_freqs), us_to_timeval(1000000ULL / baudrate));

    switch (mode) {
    case MODE_ALTERNATING_SYMBOLS:
        fsk_start(&fsk);
        while (true) {
            for (size_t i = 0; i < n_freqs; i++) {
                fsk_send_symbol(&fsk, i);
            }
        }
        break;
    case MODE_MESSAGE:
        do {
            fsk_send_sequence(&fsk, &bitstream);
            usleep(1000000UL * loop_delay);
        } while (loop);
        bitstream_destroy(&bitstream);
        break;
    case MODE_CHIRP:
        do {
            play_chirp(&tone_gen, 1000.0, 20000.0, 10.0, (struct timeval){0, 20000});
        } while (loop);
        break;
    case MODE_AUDIOFILE:
        ppbuf_init(&ppbuf, block_size * sizeof(double));
        bitbang_player_init(&bbplayer, &spinner, sndfile_info.samplerate, audio_gain);

        do {
            sf_seek(sndfile, 0, SF_SEEK_SET);
            bitbang_player_play(&bbplayer, &ppbuf);

            while (sf_read_double(sndfile, ppbuf_buf_write_begin(&ppbuf), block_size) != 0) {
                ppbuf_buf_write_done(&ppbuf);
            }

            bitbang_player_stop(&bbplayer);
            usleep(1000000UL * loop_delay);
        } while (loop);

        ppbuf_destroy(&ppbuf);
        bitbang_player_destroy(&bbplayer);
        sf_close(sndfile);
        break;
    }

    framer_destory(&framer);
    fsk_destroy(&fsk);
    simple_tone_gen_destroy(&tone_gen);
    cpu_spinner_destroy(&spinner);
    return 0;
}
