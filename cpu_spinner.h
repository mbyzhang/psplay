#ifndef __CPU_SPINNER_H__
#define __CPU_SPINNER_H__

#include <stdint.h>
#include <pthread.h>

#define CPU_SPINNER_N_CORES_ACTIVE(n) (~(~0ULL << (n)))
#define CPU_SPINNER_ALL_CORES_IDLE (0ULL)
#define CPU_SPINNER_ALL_CORES_ACTIVE (~0ULL)
#define CPU_SPINNER_CORE_MASK(n) (1ULL << (n))

typedef struct {
    int core_id;
    pthread_cond_t* cond;
    pthread_mutex_t* mutex;
    volatile uint64_t* sig;
    int shutdown_req;
} cpu_spinner_worker_args_t;

typedef struct {
    int num_cores;
    volatile uint64_t active_cores;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    pthread_t* worker_threads;
    cpu_spinner_worker_args_t* worker_args;
} cpu_spinner_t;

int cpu_spinner_init(cpu_spinner_t* spinner, int num_cores);
void cpu_spinner_spin(cpu_spinner_t* spinner, uint64_t active_cores);
void cpu_spinner_destroy(cpu_spinner_t* spinner);

#endif
