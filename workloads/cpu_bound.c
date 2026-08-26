#define _GNU_SOURCE

#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <limits.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static uint64_t monotonic_ns(void)
{
    struct timespec ts;

    if (clock_gettime(
            CLOCK_MONOTONIC,
            &ts) != 0) {

        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }

    return
        (uint64_t)ts.tv_sec * 1000000000ULL +
        (uint64_t)ts.tv_nsec;
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
    if (cpu < 0 ||
        cpu >= CPU_SETSIZE) {

        fprintf(
            stderr,
            "invalid cpu: %d\n",
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
        "  --duration-ms N  run duration in ms\n"
        "  --cpu N          pin worker to CPU N\n"
        "  --worker-id N    worker identifier\n"
        "  --help           show this help\n",
        program
    );
}

int main(
    int argc,
    char **argv)
{
    uint64_t duration_ms = 1000;
    uint64_t worker_id = 0;
    int cpu = 0;

    static const struct option options[] = {
        {
            "duration-ms",
            required_argument,
            NULL,
            'd'
        },
        {
            "cpu",
            required_argument,
            NULL,
            'c'
        },
        {
            "worker-id",
            required_argument,
            NULL,
            'i'
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
        case 'd':
            duration_ms =
                parse_u64(
                    optarg,
                    "duration-ms"
                );
            break;

        case 'c':
            cpu =
                parse_cpu(optarg);
            break;

        case 'i':
            worker_id =
                parse_u64(
                    optarg,
                    "worker-id"
                );
            break;

        case 'h':
            print_usage(argv[0]);
            return EXIT_SUCCESS;

        default:
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (duration_ms == 0) {
        fprintf(
            stderr,
            "duration-ms must be greater than zero\n"
        );

        return EXIT_FAILURE;
    }

    pin_to_cpu(cpu);

    uint64_t start_ns =
        monotonic_ns();

    uint64_t deadline_ns =
        start_ns +
        duration_ms * 1000000ULL;

    uint64_t work_units = 0;

    volatile uint64_t value =
        0x9e3779b97f4a7c15ULL ^
        worker_id;

    while (monotonic_ns() < deadline_ns) {
        for (unsigned long i = 0;
             i < 4096;
             i++) {

            value ^= value << 7;
            value ^= value >> 9;
            value *=
                0xbf58476d1ce4e5b9ULL;
            value +=
                0x94d049bb133111ebULL;
        }

        work_units++;
    }

    uint64_t end_ns =
        monotonic_ns();

    (void)value;

    printf(
        "worker_id,cpu,duration_ns,work_units\n"
    );

    printf(
        "%" PRIu64
        ",%d"
        ",%" PRIu64
        ",%" PRIu64
        "\n",
        worker_id,
        cpu,
        end_ns - start_ns,
        work_units
    );

    return EXIT_SUCCESS;
}
