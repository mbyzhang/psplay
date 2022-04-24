#include "ping_pong_buf.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "utils.h"

int ppbuf_init(ppbuf_t* ppbuf, size_t buf_size) {
    int ret = 0;

    ppbuf->buf[0] = malloc(buf_size);
    if (ppbuf->buf[0] == NULL) {
        return -1;
    }

    ppbuf->buf[1] = malloc(buf_size);
    if (ppbuf->buf[1] == NULL) {
        free(ppbuf->buf[0]);
        return -1;
    }

    memset(ppbuf->buf_state, 0, sizeof(ppbuf->buf_state));

    ppbuf->buf_readi = 0;
    ppbuf->buf_writei = 0;
    ppbuf->size = buf_size;

    CHECK_ERROR_NE0(pthread_mutex_init(&ppbuf->mutex, NULL));
    CHECK_ERROR_NE0(pthread_cond_init(&ppbuf->cond, NULL));

fail:
    return ret;
}

inline static void wait_for_state_becomes(ppbuf_t* ppbuf, int idx, int target) {
    pthread_mutex_lock(&ppbuf->mutex);
    while (ppbuf->buf_state[idx] != target) {
        pthread_cond_wait(&ppbuf->cond, &ppbuf->mutex);
    }
    pthread_mutex_unlock(&ppbuf->mutex);
}

inline static void set_state(ppbuf_t* ppbuf, int idx, int target) {
    pthread_mutex_lock(&ppbuf->mutex);
    ppbuf->buf_state[idx] = target;
    pthread_cond_signal(&ppbuf->cond);
    pthread_mutex_unlock(&ppbuf->mutex);
}

inline static int get_state(ppbuf_t* ppbuf, int idx) {
    pthread_mutex_lock(&ppbuf->mutex);
    int ret = ppbuf->buf_state[idx];
    pthread_mutex_unlock(&ppbuf->mutex);
    return ret;
}

void* ppbuf_buf_write_begin(ppbuf_t* ppbuf) {
    wait_for_state_becomes(ppbuf, ppbuf->buf_writei, PPBUF_STATE_FREE);
    return ppbuf->buf[ppbuf->buf_writei];
}

void ppbuf_buf_write_done(ppbuf_t* ppbuf) {
    set_state(ppbuf, ppbuf->buf_writei, PPBUF_STATE_ACTIVE);
    ppbuf->buf_writei = 1 - ppbuf->buf_writei;
}

void* ppbuf_buf_read_begin(ppbuf_t* ppbuf) {
    wait_for_state_becomes(ppbuf, ppbuf->buf_readi, PPBUF_STATE_ACTIVE);
    return ppbuf->buf[ppbuf->buf_readi];
}

void ppbuf_buf_read_done(ppbuf_t* ppbuf) {
    set_state(ppbuf, ppbuf->buf_readi, PPBUF_STATE_FREE);
    ppbuf->buf_readi = 1 - ppbuf->buf_readi;
}

bool ppbuf_read_available(ppbuf_t* ppbuf) {
    return get_state(ppbuf, ppbuf->buf_readi) == PPBUF_STATE_ACTIVE;
}

bool ppbuf_write_available(ppbuf_t* ppbuf) {
    return get_state(ppbuf, ppbuf->buf_writei) == PPBUF_STATE_FREE;
}

void ppbuf_destroy(ppbuf_t* ppbuf) {
    free(ppbuf->buf[0]);
    free(ppbuf->buf[1]);

    pthread_mutex_destroy(&ppbuf->mutex);
    pthread_cond_destroy(&ppbuf->cond);
}
