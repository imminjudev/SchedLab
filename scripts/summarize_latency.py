#!/usr/bin/env python3

import argparse
import csv
import math
import statistics
from pathlib import Path


def percentile(values, q):
    if not values:
        raise ValueError("no values")

    ordered = sorted(values)

    if len(ordered) == 1:
        return float(ordered[0])

    position = (len(ordered) - 1) * q
    lower = math.floor(position)
    upper = math.ceil(position)

    if lower == upper:
        return float(ordered[lower])

    weight = position - lower

    return (
        ordered[lower] * (1.0 - weight)
        + ordered[upper] * weight
    )


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("csv_file")
    args = parser.parse_args()

    path = Path(args.csv_file)

    with path.open(
        newline="",
        encoding="utf-8",
    ) as handle:
        rows = list(
            csv.DictReader(handle)
        )

    if not rows:
        raise SystemExit(
            "[FAIL] dataset contains no samples"
        )

    latencies = [
        int(row["latency_ns"])
        for row in rows
    ]

    cpus = [
        int(row["cpu"])
        for row in rows
    ]

    migrations = sum(
        1
        for previous, current
        in zip(cpus, cpus[1:])
        if previous != current
    )

    print(f"samples={len(latencies)}")
    print(
        f"mean_ns={statistics.fmean(latencies):.3f}"
    )
    print(
        f"p50_ns={percentile(latencies, 0.50):.3f}"
    )
    print(
        f"p95_ns={percentile(latencies, 0.95):.3f}"
    )
    print(
        f"p99_ns={percentile(latencies, 0.99):.3f}"
    )
    print(f"max_ns={max(latencies)}")
    print(f"cpu_migrations={migrations}")


if __name__ == "__main__":
    main()
