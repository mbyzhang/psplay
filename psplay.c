#include <stdio.h>
#include <sched.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sndfile.h>

#include "cpu_spinner.h"
#include "fsk.h"
#include "dbpsk.h"
#include "simple_tone_gen.h"
#include "framer.h"
#include "utils.h"
#include "ping_pong_buf.h"
#include "bitbang_player.h"
#include "quirks.h"

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
    dbpsk_t dbpsk;
    simple_tone_gen_t tone_gen;
    framer_t framer;
    ppbuf_t ppbuf;
    bitbang_player_t bbplayer;

    double freqs[MAX_FREQ_SIZE];
    size_t n_freqs = 0;

    int mode = MODE_ALTERNATING_SYMBOLS;
    uint8_t buf[BUF_SIZE];
    int ret;
    bitstream_t bitstream;
    framer_format_t framer_format = FRAMER_FORMAT_STANDARD;
    char* message = NULL;

    bool loop = false;
    bool use_dbpsk = false;
    double loop_delay = 0.5;

    double baudrate = 100.0;

    SNDFILE* sndfile;
    SF_INFO sndfile_info = {
        .format = 0
    };
    const int block_size = 4096;
    double audio_gain = 1.0;

    int opt;
    while ((opt = getopt(argc, argv, "lacrpm:b:d:i:g:f:")) != -1) {
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
            break;
        }
        case 'm': // message
            mode = MODE_MESSAGE;
            message = optarg;
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
        case 'p': // use DBPSK
            use_dbpsk = true;
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
        case 'r': // raw-payload frame
            framer_format = FRAMER_FORMAT_RAW_PAYLOAD;
            break;
        default:
            fprintf(stderr, "usage: %s [-aclrp] [-m message] [-i audio_file] [-f f0,f1,...] [-b baudrate] [-d loop_delay] [-i audio_gain]\n", argv[0]);
            exit(EXIT_FAILURE);
            break;
        }
    }

    if (n_freqs == 0) {
        if (use_dbpsk) {
            n_freqs = 1;
            freqs[0] = 3000.0;
        }
        else {
            n_freqs = 2;
            freqs[0] = 3000.0;
            freqs[1] = 3200.0;
        }
    }

    int m_exp = log2_int(n_freqs);

#ifdef __linux__
    linux_cpufreq_governor_set_to_performance();
#endif

    cpu_spinner_init(&spinner, 0);

    if (mode == MODE_ALTERNATING_SYMBOLS || mode == MODE_MESSAGE) {
        if (use_dbpsk) {
            if (n_freqs != 1) {
                fprintf(stderr, "DBPSK only supports one carrier frequency\n");
                exit(EXIT_FAILURE);
            }
            dbpsk_init(&dbpsk, &spinner, freqs[0], us_to_timeval(1000000ULL / baudrate));
        }
        else {
            if (!IS_POT(n_freqs) || n_freqs < 2) {
                fprintf(stderr, "Number of frequencies must be at least two and a power of two\n");
                exit(EXIT_FAILURE);
            }
            simple_tone_gen_init(&tone_gen, &spinner);
            fsk_init(&fsk, &tone_gen, freqs, m_exp, us_to_timeval(1000000ULL / baudrate));
        }
    }
    else if (mode == MODE_CHIRP) {
        simple_tone_gen_init(&tone_gen, &spinner);
    }

    switch (mode) {
    case MODE_ALTERNATING_SYMBOLS:
        if (use_dbpsk) {
            dbpsk_start(&dbpsk);
            while (true) {
                dbpsk_send_symbol(&dbpsk, 0);
                dbpsk_send_symbol(&dbpsk, 1);
            }
            dbpsk_stop(&dbpsk);
        }
        else {
            fsk_start(&fsk);
            while (true) {
                for (size_t i = 0; i < n_freqs; i++) {
                    fsk_send_symbol(&fsk, i);
                }
            }
        }
        break;
    case MODE_MESSAGE:
        framer_init(&framer, 0.2, framer_format);
        ret = bitstream_init(&bitstream, FRAME_MAX_LENGTH_BITS);
        if (ret < 0) {
            fprintf(stderr, "Could not allocate memory for bitstream\n");
            exit(EXIT_FAILURE);
        }
        ret = framer_frame(&framer, (uint8_t*)message, strlen(message), &bitstream, m_exp);
        if (ret < 0) {
            fprintf(stderr, "Could not encode message: %s\n", strerror(-ret));
            exit(EXIT_FAILURE);
        }
        bitstream_dump(&bitstream);
        do {
            if (use_dbpsk) {
                dbpsk_send_sequence(&dbpsk, &bitstream);
            }
            else {
                fsk_send_sequence(&fsk, &bitstream);
            }
            usleep(1000000UL * loop_delay);
        } while (loop);
        bitstream_destroy(&bitstream);
        framer_destory(&framer);
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

    if (mode == MODE_ALTERNATING_SYMBOLS || mode == MODE_MESSAGE) {
        if (use_dbpsk) {
            dbpsk_destroy(&dbpsk);
        }
        else {
            fsk_destroy(&fsk);
            simple_tone_gen_destroy(&tone_gen);
        }
    }
    else if (mode == MODE_CHIRP) {
        simple_tone_gen_destroy(&tone_gen);
    }

    cpu_spinner_destroy(&spinner);
    return 0;
}
