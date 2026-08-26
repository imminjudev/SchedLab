#!/usr/bin/env python3

import argparse
import csv
from pathlib import Path


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("csv_file")
    args = parser.parse_args()

    path = Path(args.csv_file)

    with path.open(
        newline="",
        encoding="utf-8",
    ) as handle:
        rows = list(csv.DictReader(handle))

    if not rows:
        print("workers=0")
        print("total_work_units=0")
        print("aggregate_work_units_per_s=0.000")
        print("fairness_jain=NA")
        return

    throughputs = []
    total_work_units = 0

    for row in rows:
        duration_ns = int(row["duration_ns"])
        work_units = int(row["work_units"])

        if duration_ns <= 0:
            raise SystemExit(
                "[FAIL] non-positive worker duration"
            )

        if work_units <= 0:
            raise SystemExit(
                "[FAIL] worker made no progress"
            )

        throughput = (
            work_units
            * 1_000_000_000.0
            / duration_ns
        )

        throughputs.append(throughput)
        total_work_units += work_units

    total_throughput = sum(throughputs)

    square_sum = sum(
        value * value
        for value in throughputs
    )

    fairness = (
        total_throughput
        * total_throughput
        / (
            len(throughputs)
            * square_sum
        )
    )

    print(f"workers={len(rows)}")
    print(
        f"total_work_units={total_work_units}"
    )
    print(
        "aggregate_work_units_per_s="
        f"{total_throughput:.3f}"
    )
    print(
        f"fairness_jain={fairness:.6f}"
    )


if __name__ == "__main__":
    main()
