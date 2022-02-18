#include "cpu_spinner.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct {
    int (*setup)(void**);
    void (*body)(void**);
    void (*teardown)(void**);
} kernel_t;

#ifdef __AVX2__
#include <immintrin.h>

static inline void kernel_avx2_naive() {
#pragma unroll(16)
    for (int i = 0; i < 16; i++) {
        asm volatile(
            "vfmadd132pd %%ymm0, %%ymm10, %%ymm11;\n"
            "vfmadd132pd %%ymm1, %%ymm10, %%ymm11;\n"
            "vfmadd132pd %%ymm2, %%ymm10, %%ymm11;\n"
            "vfmadd132pd %%ymm3, %%ymm10, %%ymm11;\n"
            "vfmadd132pd %%ymm4, %%ymm10, %%ymm11;\n"
            "vfmadd132pd %%ymm5, %%ymm10, %%ymm11;\n"
            "vfmadd132pd %%ymm6, %%ymm10, %%ymm11;\n"
            "vfmadd132pd %%ymm7, %%ymm10, %%ymm11;\n"
            "vfmadd132pd %%ymm8, %%ymm10, %%ymm11;\n"
            "vfmadd132pd %%ymm9, %%ymm10, %%ymm11;\n"
            :
            :
            : "%ymm0", "%ymm1", "%ymm2", "%ymm3", "%ymm4", "%ymm5", "%ymm6", "%ymm7", "%ymm8", "%ymm9");
    }
}

#define CPU_SPINNER_AVX2_ARR_SIZE 1024

static inline int kernel_avx2_setup(void** args) {
    *args = calloc(CPU_SPINNER_AVX2_ARR_SIZE, sizeof(__m256d));
    return (*args == NULL) ? -1 : 0;
}

static inline void kernel_avx2_body(void** args) {
    __m256d* x = *args;
    volatile __m256d a, b, c, d, e, f, g, h, q;

#pragma unroll(16)
    for (int i = 0; i < CPU_SPINNER_AVX2_ARR_SIZE; ++i) {
        __m256d t = x[i];
        a = _mm256_fmadd_pd(a, q, t);
        b = _mm256_fmadd_pd(b, q, t);
        c = _mm256_fmadd_pd(c, q, t);
        d = _mm256_fmadd_pd(d, q, t);
        e = _mm256_fmadd_pd(e, q, t);
        f = _mm256_fmadd_pd(f, q, t);
        g = _mm256_fmadd_pd(g, q, t);
        h = _mm256_fmadd_pd(h, q, t);
    }
}

static inline void kernel_avx2_teardown(void** args) {
    free(*args);
}

static kernel_t kernel_avx2 = {
    .setup = kernel_avx2_setup,
    .body = kernel_avx2_body,
    .teardown = kernel_avx2_teardown,
};

#ifndef CPU_SPINNER_KERNEL
#define CPU_SPINNER_KERNEL kernel_avx2
#endif

#endif

static inline int kernel_nop_setup(void** args) { return 0; }
static inline void kernel_nop_body(void** args) {}
static inline void kernel_nop_teardown(void** args) {}

static kernel_t kernel_nop = {
    .setup = kernel_nop_setup,
    .body = kernel_nop_body,
    .teardown = kernel_nop_teardown,
};

#ifndef CPU_SPINNER_KERNEL
#warning "AVX2 is not available, falling back to nop kernel"
#define CPU_SPINNER_KERNEL kernel_nop
#endif


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
