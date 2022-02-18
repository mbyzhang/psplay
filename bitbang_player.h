#ifndef __BITBANG_PLAYER_H__
#define __BITBANG_PLAYER_H__

#include "ping_pong_buf.h"
#include "cpu_spinner.h"
#include <event2/event.h>
#include <time.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/types.h>

#define BITBANG_PLAYER_STATE_STOPPED  0
#define BITBANG_PLAYER_STATE_PLAYING  1
#define BITBANG_PLAYER_STATE_STOPPING 2
#define BITBANG_PLAYER_STATE_STALLED  3

typedef struct {
    cpu_spinner_t* spinner;
    struct timeval sampling_interval;
    pthread_t worker_thread;
    ppbuf_t* ppbuf;
    int state;
    pthread_mutex_t state_mutex;
    double* buf;
    size_t pos;
    size_t size;
    double gain;
} bitbang_player_t;

int bitbang_player_init(bitbang_player_t* bbp, cpu_spinner_t* spinner, double fs, double gain);
int bitbang_player_play(bitbang_player_t* bbp, ppbuf_t* ppbuf);
void bitbang_player_stop(bitbang_player_t* bbp);
void bitbang_player_destroy(bitbang_player_t* bbp);

#endif
