#ifdef __linux__
#define _GNU_SOURCE
#endif

#include "cpu_spinner.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "cpu_kernels.h"
#include "utils.h"

void* worker(void* raw_arg) {
    cpu_spinner_worker_args_t* args = (cpu_spinner_worker_args_t*)raw_arg;
    void* kernel_args = NULL;
    int ret = CPU_SPINNER_KERNEL.setup(&kernel_args);
    if (ret < 0) {
        perror("Failed to setup kernel");
        exit(EXIT_FAILURE);
    }
    while (1) {
        pthread_mutex_lock(args->mutex);
        while (!(*args->sig & CPU_SPINNER_CORE_MASK(args->core_id)) && !args->shutdown_req) {
            pthread_cond_wait(args->cond, args->mutex);
        }
        pthread_mutex_unlock(args->mutex);

        while ((*args->sig & CPU_SPINNER_CORE_MASK(args->core_id)) && !args->shutdown_req) {
            CPU_SPINNER_KERNEL.body(&kernel_args);
        }

        if (args->shutdown_req) break;
    }
    CPU_SPINNER_KERNEL.teardown(&kernel_args);
    return NULL;
}

void shutdown_worker(cpu_spinner_t* spinner, int core_id) {
    spinner->worker_args[core_id].shutdown_req = true;

    pthread_mutex_lock(&spinner->mutex);
    pthread_cond_broadcast(&spinner->cond);
    pthread_mutex_unlock(&spinner->mutex);

    pthread_join(spinner->worker_threads[core_id], NULL);
}

int cpu_spinner_init(cpu_spinner_t* spinner, int num_cores) {
    int i;

    if (num_cores == 0) {
        // use all available cores
        num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    }

    spinner->num_cores = num_cores;
    spinner->active_cores = 0ULL;

    if (pthread_cond_init(&spinner->cond, NULL)) {
        goto cond_init_fail;
    }
    if (pthread_mutex_init(&spinner->mutex, NULL)) {
        goto mutex_init_fail;
    }

    spinner->worker_threads = (pthread_t*)malloc(sizeof(pthread_t) * num_cores);
    spinner->worker_args = (cpu_spinner_worker_args_t*)malloc(sizeof(cpu_spinner_worker_args_t) * num_cores);

    for (i = 0; i < num_cores; i++) {
        cpu_spinner_worker_args_t* args = &spinner->worker_args[i];
        args->cond = &spinner->cond;
        args->mutex = &spinner->mutex;
        args->sig = &spinner->active_cores;
        args->shutdown_req = false;
        args->core_id = i;

        if (pthread_create(&spinner->worker_threads[i], NULL, &worker, args)) {
            goto pthread_create_fail;
        }

#ifdef _GNU_SOURCE
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(i, &cpuset);
        CHECK_ERROR_NE0(pthread_setaffinity_np(spinner->worker_threads[i], sizeof(cpuset), &cpuset));
#endif
    }
    return 0;

pthread_create_fail:
    for (int k = i - 1; k >= 0; k--) {
        shutdown_worker(spinner, k);
    }

mutex_init_fail:
    pthread_cond_destroy(&spinner->cond);

cond_init_fail:
    return 1;
}

void cpu_spinner_spin(cpu_spinner_t* spinner, uint64_t active_cores) {
    bool has_rising_edge = false;
    bool has_falling_edge = false;

    for (int i = 0; i < spinner->num_cores; i++) {
        int old_status = !!(spinner->active_cores & CPU_SPINNER_CORE_MASK(i));
        int new_status = !!(active_cores & CPU_SPINNER_CORE_MASK(i));
        if (!old_status && new_status) {
            // rising edge
            has_rising_edge = true;
        }
        else if (old_status && !new_status) {
            // falling edge
            has_falling_edge = true;
        }
    }

    if (has_rising_edge) {
        pthread_mutex_lock(&spinner->mutex);
        spinner->active_cores = active_cores;
        pthread_cond_broadcast(&spinner->cond);
        pthread_mutex_unlock(&spinner->mutex);
    } 
    else if (has_falling_edge) {
        spinner->active_cores = active_cores;
        __sync_synchronize();
    }
}

void cpu_spinner_destroy(cpu_spinner_t* spinner) {
    for (int i = 0; i < spinner->num_cores; i++) {
        shutdown_worker(spinner, i);
    }
    free(spinner->worker_args);
    free(spinner->worker_threads);

    pthread_mutex_destroy(&spinner->mutex);
    pthread_cond_destroy(&spinner->cond);
}
