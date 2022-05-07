#include <stdio.h>
#include <sched.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
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

#define MAX_FREQ_SIZE 128

#define PARSE_CLI_DOUBLE(var) do { \
    char *endptr;                  \
    var = strtod(optarg, &endptr); \
    if (optarg == endptr) {        \
        fprintf(stderr, "Could not parse command-line argument -%c=%s\n", opt, optarg); \
        exit(EXIT_FAILURE);        \
    }                              \
} while (0);

#define PARSE_CLI_LONG(var) do {       \
    char *endptr;                      \
    var = strtol(optarg, &endptr, 10); \
    if (optarg == endptr) {            \
        fprintf(stderr, "Could not parse command-line argument -%c=%s\n", opt, optarg); \
        exit(EXIT_FAILURE);            \
    }                                  \
} while (0);

void modulator_cb(int status, cpu_spinner_t* spinner) {
    cpu_spinner_spin(spinner, (status)? CPU_SPINNER_ALL_CORES_ACTIVE : CPU_SPINNER_ALL_CORES_IDLE);
}

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
    int ret;
    bitstream_t bitstream;
    framer_format_t framer_format = FRAMER_FORMAT_STANDARD;
    double frame_ecc_level = 0.2;
    int frame_preamble_len = 5;
    uint8_t* message = malloc(FRAME_MAX_PAYLOAD_SIZE);
    CHECK_ERROR_PTR(message);
    size_t message_len = 0;

    bool use_dbpsk = false;
    double loop_delay = 0.5;
    int loop_count = 0;

    double baudrate = 100.0;

    SNDFILE* sndfile;
    SF_INFO sndfile_info = {
        .format = 0
    };
    const int block_size = 4096;
    double audio_gain = 1.0;

    static struct option long_options[] = {
        { "help",               no_argument,        NULL, 'h' },
        { "message",            required_argument,  NULL, 'm' },
        { "stdin",              no_argument,        NULL, 's' },
        { "freqs",              required_argument,  NULL, 'f' },
        { "audio-input",        required_argument,  NULL, 'i' },
        { "audio-gain",         required_argument,  NULL, 'g' },
        { "baud-rate",          required_argument,  NULL, 'b' },
        { "loop-delay",         required_argument,  NULL, 'd' },
        { "loop-count",         required_argument,  NULL, 'n' },
        { "dbpsk",              no_argument,        NULL, 'p' },
        { "mode-chirp",         no_argument,        NULL, 'c' },
        { "mode-alternating",   no_argument,        NULL, 'a' },
        { "frame-payload-raw",  no_argument,        NULL, 'r' },
        { "frame-ecc-level",    required_argument,  NULL, 'e' },
        { "frame-preamble-len", required_argument,  NULL, 'q' },
        { NULL,                 0,                  NULL, 0   }
    };
    const char* short_options = "hm:sf:i:g:b:d:n:pcare:q:";
    int opt;

    while ((opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
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
        case 's': // read message from stdin
            mode = MODE_MESSAGE;
            while (FRAME_MAX_PAYLOAD_SIZE - message_len > 0) {
                int bytes_read = read(STDIN_FILENO, message + message_len, FRAME_MAX_PAYLOAD_SIZE - message_len);

                if (bytes_read == 0) {
                    break;
                }
                else if (bytes_read < 0) {
                    perror("read");
                    abort();
                }

                message_len += bytes_read;
            }
            break;
        case 'm': // message
            mode = MODE_MESSAGE;
            message_len = strlen(optarg);
            if (message_len > FRAME_MAX_PAYLOAD_SIZE) {
                fprintf(stderr, "Input message too long\n");
                exit(EXIT_FAILURE);
            }
            else {
                memcpy(message, optarg, message_len);
            }
 
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
        case 'n': // loop count
            PARSE_CLI_LONG(loop_count);
            break;
        case 'e': // ECC level
            PARSE_CLI_DOUBLE(frame_ecc_level);
            break;
        case 'q': // frame preamble length
            PARSE_CLI_LONG(frame_preamble_len);
            break;
        case 'h': // help
        default:
            fprintf(stderr,
                "OVERVIEW: Transmit message over sound or play audio using processor VRMs\n"
                "USAGE: %s [options]\n"
                "\n"
                "OPTIONS:\n"
                "\n"
                "  -h --help                            show this message\n"
                "  -m --message     <message>           message to transmit\n"
                "  -s --stdin                           read message from stdin\n"
                "  -f --freqs       <f1>[,<f2>[,...]]   carrier frequencies\n"
                "  -i --audio-input <file>              play audio from <file>\n"
                "  -g --audio-gain  <value>             audio playback gain [default=1.0]\n"
                "  -b --baud-rate   <value>             baud rate [default=100.0]\n"
                "  -d --loop-delay  <value>             loop delay in seconds [default=0.5]\n"
                "  -n --loop-count  <n>                 loop count [default=1]\n"
                "  -p --dbpsk                           use DBPSK modulation instead of FSK\n"
                "  -c --mode-chirp                      play chirp test sound pattern\n"
                "  -a --mode-alternating                transmit alternating symbols [default]\n"
                "  -r --frame-payload-raw               send payload in raw (without line coding and ECC)\n"
                "  -e --frame-ecc-level      <value>    level of payload redundancy [default=0.2]\n"
                "  -q --frame-preamble-len   <n>        length of frame preamble [default=5]"
                "\n",
                argv[0]);
            exit(EXIT_FAILURE);
            break;
        }
    }

    if (loop_count == 0) {
        if (mode == MODE_ALTERNATING_SYMBOLS) loop_count = -1;
        else loop_count = 1;
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
            dbpsk_init(&dbpsk, freqs[0], hz_to_period_timespec(baudrate), (void(*)(int, void*))modulator_cb, &spinner);
        }
        else {
            if (!IS_POT(n_freqs) || n_freqs < 2) {
                fprintf(stderr, "Number of frequencies must be at least two and a power of two\n");
                exit(EXIT_FAILURE);
            }
            fsk_init(&fsk, freqs, m_exp, hz_to_period_timespec(baudrate), (void(*)(int, void*))modulator_cb, &spinner);
        }
    }
    else if (mode == MODE_CHIRP) {
        simple_tone_gen_init(&tone_gen, &spinner);
    }

    switch (mode) {
    case MODE_ALTERNATING_SYMBOLS:
        if (use_dbpsk) {
            dbpsk_start(&dbpsk);
            while (loop_count == -1 || loop_count-- > 0) {
                dbpsk_send_symbol(&dbpsk, 0);
                dbpsk_send_symbol(&dbpsk, 1);
            }
            dbpsk_stop(&dbpsk);
        }
        else {
            fsk_start(&fsk);
            while (loop_count == -1 || loop_count-- > 0) {
                for (size_t i = 0; i < n_freqs; i++) {
                    fsk_send_symbol(&fsk, i);
                }
            }
            fsk_stop(&fsk);
        }
        break;
    case MODE_MESSAGE:
        framer_init(&framer, frame_ecc_level, framer_format, frame_preamble_len, m_exp);
        ret = bitstream_init(&bitstream, FRAME_MAX_LENGTH_BITS);
        if (ret < 0) {
            fprintf(stderr, "Could not allocate memory for bitstream\n");
            exit(EXIT_FAILURE);
        }
        ret = framer_frame(&framer, message, message_len, &bitstream);
        if (ret < 0) {
            fprintf(stderr, "Could not encode message: %s\n", strerror(-ret));
            exit(EXIT_FAILURE);
        }
        bitstream_dump(&bitstream);
        while (true) {
            bitstream_seek(&bitstream, 0);
            if (use_dbpsk) {
                dbpsk_send_sequence(&dbpsk, &bitstream);
            }
            else {
                fsk_send_sequence(&fsk, &bitstream);
            }
            if (loop_count != -1 && --loop_count <= 0) break;
            usleep(1000000UL * loop_delay);
        }
        bitstream_destroy(&bitstream);
        framer_destory(&framer);
        break;
    case MODE_CHIRP:
        while (true) {
            play_chirp(&tone_gen, 1000.0, 20000.0, 10.0, (struct timespec){0, 20000000L});
            if (loop_count != -1 && --loop_count <= 0) break;
            usleep(1000000UL * loop_delay);
        }
        break;
    case MODE_AUDIOFILE:
        ppbuf_init(&ppbuf, block_size * sizeof(double));
        bitbang_player_init(&bbplayer, &spinner, sndfile_info.samplerate, audio_gain);

        while (true) {
            sf_seek(sndfile, 0, SF_SEEK_SET);
            bitbang_player_play(&bbplayer, &ppbuf);

            while (sf_read_double(sndfile, ppbuf_buf_write_begin(&ppbuf), block_size) != 0) {
                ppbuf_buf_write_done(&ppbuf);
            }

            bitbang_player_stop(&bbplayer);
            if (loop_count != -1 && --loop_count <= 0) break;
            usleep(1000000UL * loop_delay);
        }

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
        }
    }
    else if (mode == MODE_CHIRP) {
        simple_tone_gen_destroy(&tone_gen);
    }

    free(message);
    cpu_spinner_destroy(&spinner);
    return 0;
}
