#!/usr/bin/env python3

from pathlib import Path
import csv
import subprocess


WORKER_COUNTS = [0, 1, 2, 4]

OUTPUT_ROOT = Path("build/baseline-sweep")


def run_case(worker_count):
    output_dir = OUTPUT_ROOT / f"workers-{worker_count}"

    command = [
        "./scripts/run_contention_case.sh",
        str(worker_count),
        str(output_dir),
        "0",
        "200",
        "20",
        "5000",
        "250",
    ]

    subprocess.run(
        command,
        check=True,
    )

def read_summary(path):
    values = {}

    with path.open(
        encoding="utf-8",
    ) as file:
        for line in file:
            line = line.strip()

            if "=" not in line:
                continue

            key, value = line.split(
                "=",
                1,
            )

            values[key] = value

    return values

def build_row(worker_count):
    case_dir = OUTPUT_ROOT / f"workers-{worker_count}"

    latency = read_summary(
        case_dir / "latency_summary.txt"
    )

    throughput = read_summary(
        case_dir / "throughput_summary.txt"
    )

    return {
        "workers": worker_count,
        "mean_ns": latency["mean_ns"],
        "p50_ns": latency["p50_ns"],
        "p95_ns": latency["p95_ns"],
        "p99_ns": latency["p99_ns"],
        "max_ns": latency["max_ns"],
        "cpu_migrations": latency["cpu_migrations"],
        "throughput": throughput[
            "aggregate_work_units_per_s"
        ],
        "fairness": throughput["fairness_jain"],
    }

def write_summary():
    rows = []

    for worker_count in WORKER_COUNTS:
        rows.append(
            build_row(worker_count)
        )

    output_path = OUTPUT_ROOT / "summary.csv"

    fieldnames = [
        "workers",
        "mean_ns",
        "p50_ns",
        "p95_ns",
        "p99_ns",
        "max_ns",
        "cpu_migrations",
        "throughput",
        "fairness",
    ]

    with output_path.open(
        "w",
        newline="",
        encoding="utf-8",
    ) as file:
        writer = csv.DictWriter(
            file,
            fieldnames=fieldnames,
        )

        writer.writeheader()
        writer.writerows(rows)

def main():
    OUTPUT_ROOT.mkdir(
        parents=True,
        exist_ok=True,
    )

    for worker_count in WORKER_COUNTS:
        print(
            f"\n=== workers={worker_count} ==="
        )

        run_case(worker_count)

    write_summary()

    print(
        f"\nsummary: {OUTPUT_ROOT / 'summary.csv'}"
    )


if __name__ == "__main__":
    main()