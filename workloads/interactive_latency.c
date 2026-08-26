#define _GNU_SOURCE

#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <limits.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct sample {
    uint64_t target_ns;
    uint64_t actual_ns;
    uint64_t latency_ns;
    int cpu;
};

static uint64_t timespec_to_ns(
    const struct timespec *ts)
{
    return
        (uint64_t)ts->tv_sec * 1000000000ULL +
        (uint64_t)ts->tv_nsec;
}

static struct timespec ns_to_timespec(
    uint64_t ns)
{
    struct timespec ts;

    ts.tv_sec =
        (time_t)(ns / 1000000000ULL);

    ts.tv_nsec =
        (long)(ns % 1000000000ULL);

    return ts;
}

static uint64_t monotonic_ns(void)
{
    struct timespec ts;

    if (clock_gettime(
            CLOCK_MONOTONIC,
            &ts) != 0) {

        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }

    return timespec_to_ns(&ts);
}

static void busy_for_ns(
    uint64_t duration_ns)
{
    if (duration_ns == 0) {
        return;
    }

    uint64_t deadline =
        monotonic_ns() + duration_ns;

    volatile uint64_t value =
        0x9e3779b97f4a7c15ULL;

    while (monotonic_ns() < deadline) {
        value ^= value << 7;
        value ^= value >> 9;
        value *= 0xbf58476d1ce4e5b9ULL;
    }

    (void)value;
}

static uint64_t parse_u64(
    const char *text,
    const char *name)
{
    char *end = NULL;
    errno = 0;

    unsigned long long value =
        strtoull(
            text,
            &end,
            10
        );

    if (errno != 0 ||
        end == text ||
        *end != '\0') {

        fprintf(
            stderr,
            "invalid %s: %s\n",
            name,
            text
        );

        exit(EXIT_FAILURE);
    }

    return (uint64_t)value;
}

static int parse_cpu(
    const char *text)
{
    uint64_t value =
        parse_u64(
            text,
            "cpu"
        );

    if (value > INT_MAX) {
        fprintf(
            stderr,
            "cpu value too large\n"
        );

        exit(EXIT_FAILURE);
    }

    return (int)value;
}

static void pin_to_cpu(
    int cpu)
{
    if (cpu < 0) {
        return;
    }

    if (cpu >= CPU_SETSIZE) {
        fprintf(
            stderr,
            "cpu %d exceeds CPU_SETSIZE\n",
            cpu
        );

        exit(EXIT_FAILURE);
    }

    cpu_set_t set;

    CPU_ZERO(&set);
    CPU_SET(cpu, &set);

    if (sched_setaffinity(
            0,
            sizeof(set),
            &set) != 0) {

        perror("sched_setaffinity");
        exit(EXIT_FAILURE);
    }
}

static void print_usage(
    const char *program)
{
    fprintf(
        stderr,
        "Usage: %s [options]\n"
        "\n"
        "Options:\n"
        "  --iterations N   measured samples (default: 1000)\n"
        "  --warmup N       ignored warmup samples (default: 100)\n"
        "  --period-us N    release period in us (default: 10000)\n"
        "  --burst-us N     CPU burst after wake in us (default: 500)\n"
        "  --cpu N          pin to CPU N (default: no pinning)\n"
        "  --help           show this help\n",
        program
    );
}

int main(
    int argc,
    char **argv)
{
    uint64_t iterations = 1000;
    uint64_t warmup = 100;
    uint64_t period_us = 10000;
    uint64_t burst_us = 500;
    int cpu = -1;

    static const struct option options[] = {
        {
            "iterations",
            required_argument,
            NULL,
            'i'
        },
        {
            "warmup",
            required_argument,
            NULL,
            'w'
        },
        {
            "period-us",
            required_argument,
            NULL,
            'p'
        },
        {
            "burst-us",
            required_argument,
            NULL,
            'b'
        },
        {
            "cpu",
            required_argument,
            NULL,
            'c'
        },
        {
            "help",
            no_argument,
            NULL,
            'h'
        },
        {
            NULL,
            0,
            NULL,
            0
        }
    };

    for (;;) {
        int option =
            getopt_long(
                argc,
                argv,
                "",
                options,
                NULL
            );

        if (option == -1) {
            break;
        }

        switch (option) {
        case 'i':
            iterations =
                parse_u64(
                    optarg,
                    "iterations"
                );
            break;

        case 'w':
            warmup =
                parse_u64(
                    optarg,
                    "warmup"
                );
            break;

        case 'p':
            period_us =
                parse_u64(
                    optarg,
                    "period-us"
                );
            break;

        case 'b':
            burst_us =
                parse_u64(
                    optarg,
                    "burst-us"
                );
            break;

        case 'c':
            cpu =
                parse_cpu(optarg);
            break;

        case 'h':
            print_usage(argv[0]);
            return EXIT_SUCCESS;

        default:
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (iterations == 0) {
        fprintf(
            stderr,
            "iterations must be greater than zero\n"
        );
        return EXIT_FAILURE;
    }

    if (period_us == 0) {
        fprintf(
            stderr,
            "period-us must be greater than zero\n"
        );
        return EXIT_FAILURE;
    }

    if (burst_us >= period_us) {
        fprintf(
            stderr,
            "burst-us must be smaller than period-us\n"
        );
        return EXIT_FAILURE;
    }

    if (iterations >
        SIZE_MAX / sizeof(struct sample)) {

        fprintf(
            stderr,
            "sample count too large\n"
        );
        return EXIT_FAILURE;
    }

    struct sample *samples =
        calloc(
            (size_t)iterations,
            sizeof(*samples)
        );

    if (samples == NULL) {
        perror("calloc");
        return EXIT_FAILURE;
    }

    pin_to_cpu(cpu);

    uint64_t period_ns =
        period_us * 1000ULL;

    uint64_t burst_ns =
        burst_us * 1000ULL;

    uint64_t total =
        warmup + iterations;

    uint64_t start_ns =
        monotonic_ns() +
        100000000ULL;

    fprintf(
        stderr,
        "iterations=%" PRIu64
        " warmup=%" PRIu64
        " period_us=%" PRIu64
        " burst_us=%" PRIu64
        " cpu=%d\n",
        iterations,
        warmup,
        period_us,
        burst_us,
        cpu
    );

    for (uint64_t sequence = 0;
         sequence < total;
         sequence++) {

        uint64_t target_ns =
            start_ns +
            sequence * period_ns;

        struct timespec target =
            ns_to_timespec(target_ns);

        int rc;

        do {
            rc =
                clock_nanosleep(
                    CLOCK_MONOTONIC,
                    TIMER_ABSTIME,
                    &target,
                    NULL
                );
        } while (rc == EINTR);

        if (rc != 0) {
            fprintf(
                stderr,
                "clock_nanosleep: %s\n",
                strerror(rc)
            );

            free(samples);
            return EXIT_FAILURE;
        }

        uint64_t actual_ns =
            monotonic_ns();

        int running_cpu =
            sched_getcpu();

        if (running_cpu < 0) {
            perror("sched_getcpu");
            free(samples);
            return EXIT_FAILURE;
        }

        if (sequence >= warmup) {
            uint64_t index =
                sequence - warmup;

            samples[index].target_ns =
                target_ns;

            samples[index].actual_ns =
                actual_ns;

            samples[index].latency_ns =
                actual_ns >= target_ns
                    ? actual_ns - target_ns
                    : 0;

            samples[index].cpu =
                running_cpu;
        }

        busy_for_ns(burst_ns);
    }

    printf(
        "sample,target_ns,actual_ns,"
        "latency_ns,cpu\n"
    );

    for (uint64_t i = 0;
         i < iterations;
         i++) {

        printf(
            "%" PRIu64
            ",%" PRIu64
            ",%" PRIu64
            ",%" PRIu64
            ",%d\n",
            i,
            samples[i].target_ns,
            samples[i].actual_ns,
            samples[i].latency_ns,
            samples[i].cpu
        );
    }

    free(samples);

    return EXIT_SUCCESS;
}
