#ifndef __CPU_KERNELS__
#define __CPU_KERNELS__

typedef struct {
    int (*setup)(void**);
    void (*body)(void**);
    void (*teardown)(void**);
} kernel_t;

#ifdef __AVX2__
#include <stdlib.h>
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

#define CPU_KERNEL_AVX2_ARR_SIZE 1024

static inline int kernel_avx2_setup(void** args) {
    *args = calloc(CPU_KERNEL_AVX2_ARR_SIZE, sizeof(__m256d));
    return (*args == NULL) ? -1 : 0;
}

static inline void kernel_avx2_body(void** args) {
    __m256d* x = *args;
    volatile __m256d a, b, c, d, e, f, g, h, q;

#pragma unroll(16)
    for (int i = 0; i < CPU_KERNEL_AVX2_ARR_SIZE; ++i) {
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

#endif
