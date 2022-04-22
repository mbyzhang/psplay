#include "dbpsk.h"
#include "utils.h"

#include <stdbool.h>
#include <math.h>

void freq_gen_cb(evutil_socket_t fd, short what, void *raw_arg) {
    dbpsk_t* dbpsk = (dbpsk_t*)raw_arg;

    pthread_mutex_lock(&dbpsk->status_mutex);
    dbpsk->status = !dbpsk->status;
    pthread_mutex_unlock(&dbpsk->status_mutex);

    cpu_spinner_spin(dbpsk->spinner, (dbpsk->status)? CPU_SPINNER_ALL_CORES_ACTIVE : CPU_SPINNER_ALL_CORES_IDLE);
}

static void* worker(void* raw_arg) {
    dbpsk_t* dbpsk = (dbpsk_t*)raw_arg;
    thread_set_priority_to_max();
    event_base_dispatch(dbpsk->event_base);

    return NULL;
}

int dbpsk_init(dbpsk_t* dbpsk, cpu_spinner_t* spinner, double carrier_freq, struct timeval symbol_duration) {
    dbpsk->spinner = spinner;
    dbpsk->event_base = event_base_new();
    dbpsk->status = 0;
    dbpsk->last_symbol = 0;
    dbpsk->carrier_freq = carrier_freq;
    dbpsk->symbol_duration_ts = timeval_to_timespec(symbol_duration);
    dbpsk->half_period_tv = us_to_timeval(round(0.5 / dbpsk->carrier_freq * 1e6));

    ASSERT_SUCCESS_NZ(pthread_mutex_init(&dbpsk->status_mutex, NULL));

    thread_set_priority_to_max();

    return 0;
}

int dbpsk_start(dbpsk_t* dbpsk) {
    dbpsk->ev_freq_gen = event_new(dbpsk->event_base, -1, EV_PERSIST, freq_gen_cb, dbpsk);
    event_add(dbpsk->ev_freq_gen, &dbpsk->half_period_tv);
    clock_gettime(CLOCK_MONOTONIC_RAW, &dbpsk->last_symbol_time_ts);

    ASSERT_SUCCESS_Z(dbpsk->ev_freq_gen);
    ASSERT_SUCCESS_NZ(pthread_create(&dbpsk->worker, NULL, &worker, dbpsk));

    return 0;
}

int dbpsk_send_symbol(dbpsk_t* dbpsk, unsigned int symbol) {
    if (symbol != dbpsk->last_symbol) {
        pthread_mutex_lock(&dbpsk->status_mutex);
        dbpsk->status = !dbpsk->status;
        pthread_mutex_unlock(&dbpsk->status_mutex);
    }

    dbpsk->last_symbol = symbol;

    timespec_step(&dbpsk->last_symbol_time_ts, dbpsk->symbol_duration_ts);
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);
    timespec_diff(&now, dbpsk->last_symbol_time_ts);
    nanosleep(&now, NULL);

    return 0;
}

int dbpsk_stop(dbpsk_t* dbpsk) {
    event_del(dbpsk->ev_freq_gen);
    ASSERT_SUCCESS_NZ(pthread_join(dbpsk->worker, NULL));
    return 0;
}

ssize_t dbpsk_send_sequence(dbpsk_t* dbpsk, bitstream_t* s) {
    dbpsk_start(dbpsk);

    ssize_t tx_total_bits = 0;
    
    while (true) {
        unsigned int symbol = 0;
        ssize_t n_bits = bitstream_read(s, &symbol, 1);
        if (n_bits == 0) break;
        else if (n_bits < 0) return n_bits;
        tx_total_bits += n_bits;
        dbpsk_send_symbol(dbpsk, symbol);
    }

    dbpsk_stop(dbpsk);

    return tx_total_bits;
}

void dbpsk_destroy(dbpsk_t* dbpsk) {
    event_base_free(dbpsk->event_base);
    pthread_mutex_destroy(&dbpsk->status_mutex);
}
