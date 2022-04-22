#ifndef __DBPSK_H__
#define __DBPSK_H__

#include <sys/time.h>
#include <pthread.h>
#include <event2/event.h>

#include "cpu_spinner.h"
#include "bitstream.h"

typedef struct {
    cpu_spinner_t* spinner;
    int status;
    int last_symbol;
    struct timespec last_symbol_time_ts;
    pthread_mutex_t status_mutex;
    pthread_t worker;
    struct event_base* event_base;
    struct event* ev_freq_gen;
    double carrier_freq;
    struct timespec symbol_duration_ts;
    struct timeval half_period_tv;
} dbpsk_t;

int dbpsk_init(dbpsk_t* dbpsk, cpu_spinner_t* spinner, double carrier_freq, struct timeval symbol_duration);
int dbpsk_start(dbpsk_t* dbpsk);
int dbpsk_send_symbol(dbpsk_t* dbpsk, unsigned int symbol);
int dbpsk_stop(dbpsk_t* dbpsk);
ssize_t dbpsk_send_sequence(dbpsk_t* dbpsk, bitstream_t* s);
void dbpsk_destroy(dbpsk_t* dbpsk);

#endif