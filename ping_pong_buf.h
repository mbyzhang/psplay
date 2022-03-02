#ifndef __PING_PONG_BUF_H__
#define __PING_PONG_BUF_H__

#include <sys/types.h>
#include <stdbool.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PPBUF_STATE_FREE     0
#define PPBUF_STATE_ACTIVE   1

typedef struct {
    void* buf[2];
    size_t size;
    int buf_state[2];
    int buf_readi;
    int buf_writei;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} ppbuf_t;


int ppbuf_init(ppbuf_t* ppbuf, size_t buf_size);

void* ppbuf_buf_write_begin(ppbuf_t* ppbuf);
void ppbuf_buf_write_done(ppbuf_t* ppbuf);

void* ppbuf_buf_read_begin(ppbuf_t* ppbuf);
void ppbuf_buf_read_done(ppbuf_t* ppbuf);

bool ppbuf_read_available(ppbuf_t* ppbuf);
bool ppbuf_write_available(ppbuf_t* ppbuf);

void ppbuf_destroy(ppbuf_t* ppbuf);

#ifdef __cplusplus
}
#endif

#endif
