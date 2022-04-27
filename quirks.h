#include <stdio.h>
#include <string.h>

// Linux-specific quirks
#ifdef __linux__
#include <sys/sysinfo.h>

static int file_read(const char* filename, char* buf, size_t maxlen) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) return -1;
    char* ret = fgets(buf, maxlen, fp);
    if (ret != NULL) buf[strcspn(buf, "\n")] = 0;
    fclose(fp);
    return 1;
}

static int file_write(const char* filename, const char* buf) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) return -1;
    setvbuf(fp, NULL, _IONBF, 0);
    int ret = fprintf(fp, "%s\n", buf);
    fclose(fp);
    return ret;
}

static void linux_cpufreq_governor_set_to_performance() {
    const char* governor_target = "performance";
    char filename[256];
    char governor_current[256];
    char scaling_driver[256];
    int ret;
    int n_procs = get_nprocs();
    for (int i = 0; i < n_procs; ++i) {
        snprintf(filename, sizeof(filename), "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_driver", i);
        ret = file_read(filename, scaling_driver, sizeof(scaling_driver));
        if (ret <= 0) {
            perror("quirks: Failed to get current CPUFreq scaling driver");
            continue;
        }
        if (strncmp(scaling_driver, "intel_pstate", sizeof(scaling_driver)) != 0) {
            fprintf(stderr, "quirks: Unsupported CPUFreq scaling driver \"%s\" for CPU %d\n", scaling_driver, i);
            continue;
        }

        snprintf(filename, sizeof(filename), "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_governor", i);

        ret = file_read(filename, governor_current, sizeof(governor_current));
        if (ret <= 0) {
            perror("quirks: Failed to get current CPUFreq scaling governor");
            continue;
        }
        if (strncmp(governor_current, governor_target, sizeof(governor_current)) != 0) {
            fprintf(stderr, "quirks: Setting CPUFreq scaling governor for CPU %d from %s to %s\n", i, governor_current, governor_target);
            ret = file_write(filename, governor_target);
            if (ret <= 0) {
                perror("quirks: Failed to set CPUFreq scaling governor, performance may be affected");
                continue;
            }
        }
    }
}

#endif
