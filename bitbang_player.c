#include "bitbang_player.h"
#include "utils.h"
#include "ftimer.h"
#include <limits.h>
#include <math.h>
#include <errno.h>
#include <string.h>

int bitbang_player_init(bitbang_player_t* bbp, cpu_spinner_t* spinner, double fs, double gain) {
    bbp->spinner = spinner;
    bbp->sampling_interval = hz_to_period_timespec(fs);
    bbp->state = BITBANG_PLAYER_STATE_STOPPED;
    bbp->gain = gain;
    pthread_mutex_init(&bbp->state_mutex, NULL);

    fprintf(stderr, "bitbang_player: initialized with fs=%f\n", fs);
    return 0;
}

typedef struct {
    bitbang_player_t* bbp;
    ftimer_t timer;
} tick_cb_args_t;

static void tick_cb_func(void *args_raw) {
    bitbang_player_t* bbp = (bitbang_player_t*)args_raw;

    if (bbp->size == bbp->pos) {
        if (bbp->buf != NULL) {
            ppbuf_buf_read_done(bbp->ppbuf);
            bbp->buf = NULL;
        }

        pthread_mutex_lock(&bbp->state_mutex);
        if (ppbuf_read_available(bbp->ppbuf)) {
            bbp->buf = ppbuf_buf_read_begin(bbp->ppbuf);
            bbp->pos = 0;

            if (bbp->state == BITBANG_PLAYER_STATE_STALLED) {
                // STALLED -> PLAYING
                bbp->state = BITBANG_PLAYER_STATE_PLAYING;
                fprintf(stderr, "bitbang_player: playback state STALLED -> PLAYING\n");
            }
        }
        else {
            switch (bbp->state) {
            case BITBANG_PLAYER_STATE_STOPPING:
                // STOPPING -> STOPPED
                bbp->state = BITBANG_PLAYER_STATE_STOPPED;
                fprintf(stderr, "bitbang_player: playback state STOPPING -> STOPPED\n");
                cpu_spinner_spin(bbp->spinner, CPU_SPINNER_ALL_CORES_IDLE);
                ftimer_exit(&bbp->timer);
                break;
            case BITBANG_PLAYER_STATE_PLAYING:
                // PLAYING -> STALLED
                bbp->state = BITBANG_PLAYER_STATE_STALLED;
                fprintf(stderr, "bitbang_player: playback state PLAYING -> STALLED\n");

                // stop spinning all cores when the playback is stalled to save energy
                cpu_spinner_spin(bbp->spinner, CPU_SPINNER_ALL_CORES_IDLE);
                break;
            }
        }
        pthread_mutex_unlock(&bbp->state_mutex);
    }

    if (bbp->pos < bbp->size) {
        double sample = bbp->buf[bbp->pos];

        int ncores = round((sample * bbp->gain + 1.0) * bbp->spinner->num_cores / 2.0);
        // printf("sample = %f, ncores = %d\n", sample, ncores);
        if (ncores < 0) ncores = 0;
        cpu_spinner_spin(bbp->spinner, CPU_SPINNER_N_CORES_ACTIVE(ncores));
        ++bbp->pos;
    }
}

int bitbang_player_play(bitbang_player_t* bbp, ppbuf_t* ppbuf) {
    bbp->ppbuf = ppbuf;
    bbp->state = BITBANG_PLAYER_STATE_STALLED;
    bbp->size = ppbuf->size / sizeof(double);
    bbp->pos = bbp->size;

    ftimer_create(&bbp->timer, FTIMER_RUN_ASYNC | FTIMER_RUN_RT, bbp->sampling_interval, tick_cb_func, bbp);

    return 0;
}

void bitbang_player_stop(bitbang_player_t* bbp) {
    if (bbp->state == BITBANG_PLAYER_STATE_STALLED) {
        fprintf(stderr, "bitbang_player: playback state STALLED -> STOPPING\n");
    }
    else if (bbp->state == BITBANG_PLAYER_STATE_PLAYING) {
        fprintf(stderr, "bitbang_player: playback state PLAYING -> STOPPING\n");
    }
    else {
        return;
    }
    
    pthread_mutex_lock(&bbp->state_mutex);
    bbp->state = BITBANG_PLAYER_STATE_STOPPING;
    pthread_mutex_unlock(&bbp->state_mutex);
    ftimer_destroy(&bbp->timer);
}

void bitbang_player_destroy(bitbang_player_t* bbp) {
    pthread_mutex_destroy(&bbp->state_mutex);
}
