#include <stdio.h>
#include <pthread.h>
#include <sched.h>

#include "cpu_spinner.h"
#include "fsk.h"
#include "simple_tone_gen.h"

int main(int argc, char* argv[]) {
    cpu_spinner_t spinner;
    fsk_t fsk;
    simple_tone_gen_t tone_gen;

    const int policy = SCHED_RR;
    struct sched_param param = {
        .sched_priority = sched_get_priority_max(policy)
    };
    pthread_setschedparam(pthread_self(), policy, &param);

    cpu_spinner_init(&spinner, 0);
    simple_tone_gen_init(&tone_gen, &spinner);
    fsk_init(&fsk, &tone_gen, 3000, 3100, (struct timeval){0, 100000});

    for (int i = 0; i < 100; i++) {
        fsk_send_symbol(&fsk, i % 2);
    }

    fsk_destroy(&fsk);
    simple_tone_gen_destroy(&tone_gen);
    cpu_spinner_destroy(&spinner);
    return 0;
}
