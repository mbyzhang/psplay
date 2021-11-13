#include <stdio.h>
#include <pthread.h>
#include <sched.h>

#include "cpu_spinner.h"
#include "fsk.h"

int main(int argc, char* argv[]) {
    cpu_spinner_t spinner;
    fsk_t fsk;

    const int policy = SCHED_RR;
    struct sched_param param = {
        .sched_priority = sched_get_priority_max(policy)
    };
    pthread_setschedparam(pthread_self(), policy, &param);

    cpu_spinner_init(&spinner, 0);
    fsk_init(&fsk, &spinner);

    for (int j = 0; j < 20; j++) {
        for (double i = 300.0; i < 10000.0; i += 10.0) {
            fsk_play_tone(&fsk, i, (struct timeval){ 0, 30000 });
        }
    }

    fsk_destroy(&fsk);
    cpu_spinner_destroy(&spinner);
    return 0;
}
