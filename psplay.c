#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#include "cpu_spinner.h"
#include "fsk.h"
#include "simple_tone_gen.h"
#include "framer.h"
#include "utils.h"

#define MODE_ALTERNATING_SYMBOLS 0
#define MODE_MESSAGE 1
#define MODE_CHIRP 2
#define BUF_SIZE 1024

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

    const int policy = SCHED_RR;
    struct sched_param param = {
        .sched_priority = sched_get_priority_max(policy)
    };
    pthread_setschedparam(pthread_self(), policy, &param);

    cpu_spinner_init(&spinner, 0);
    simple_tone_gen_init(&tone_gen, &spinner);

    double fsk_f1 = 3000.0;
    double fsk_f0 = 3200.0;

    int mode = MODE_ALTERNATING_SYMBOLS;
    uint8_t buf[BUF_SIZE];
    int buf_len = 0;

    bool loop = false;
    double loop_delay = 0.5;

    double baudrate = 100.0;

    int opt;
    while ((opt = getopt(argc, argv, ":0:1:m:lacb:d:")) != -1) {
        switch (opt) {
        case '0': // f0/f1
            PARSE_CLI_DOUBLE(fsk_f0);
            break;
        case '1':
            PARSE_CLI_DOUBLE(fsk_f1);
            break;
        case 'm': // message
            mode = MODE_MESSAGE;
            buf_len = framer_frame((uint8_t*)optarg, strlen(optarg), buf, sizeof(buf));
            if (buf_len < 0) {
                fprintf(stderr, "Could not encode message: %s\n", strerror(-buf_len));
                exit(EXIT_FAILURE);
            }
            break;
        case 'l': // loop
            loop = true;
            break;
        case 'a': // alternating symbols
            mode = MODE_ALTERNATING_SYMBOLS;
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
            fprintf(stderr, "usage: %s [-acl] [-m message] [-0 fsk_f0] [-1 fsk_f1] [-b baudrate] [-d loop_delay]\n", argv[0]);
            exit(EXIT_FAILURE);
            break;
        }
    }

    fsk_init(&fsk, &tone_gen, fsk_f1, fsk_f0, us_to_timeval(1000000ULL / baudrate));

    switch (mode) {
    case MODE_ALTERNATING_SYMBOLS:
        fsk_start(&fsk);
        while (true) {
            fsk_send_symbol(&fsk, 0);
            fsk_send_symbol(&fsk, 1);
        }
        break;
    case MODE_MESSAGE:
        do {
            fsk_send_sequence(&fsk, buf, buf_len);
            usleep(1000000UL * loop_delay);
        } while (loop);
        break;
    case MODE_CHIRP:
        do {
            play_chirp(&tone_gen, 1000.0, 20000.0, 10.0, (struct timeval){0, 20000});
        } while (loop);
        break;
    }

    fsk_destroy(&fsk);
    simple_tone_gen_destroy(&tone_gen);
    cpu_spinner_destroy(&spinner);
    return 0;
}
