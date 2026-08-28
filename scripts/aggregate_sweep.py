#!/usr/bin/env python3

from pathlib import Path
import csv
import statistics

INPUT_PATH = Path(
    "build/baseline-sweep/summary.csv"
)

OUTPUT_PATH = Path(
    "build/baseline-sweep/aggregate.csv"
)

def read_rows():
    with INPUT_PATH.open(
        newline="",
        encoding="utf-8",
    ) as file:
        return list(
            csv.DictReader(file)
        )

def group_by_workers(rows):
    groups = {}

    for row in rows:
        workers = int(row["workers"])

        if workers not in groups:
            groups[workers] = []

        groups[workers].append(row)

    return groups

def summarize_group(runs):
    p99_values = []

    throughput_values = []

    for run in runs:
        throughput_values.append(
            float(run["throughput"])
        )

    fairness_values = []

    for run in runs:
        if run["fairness"] != "NA":
            fairness_values.append(
                float(run["fairness"])
            )

    for run in runs:
        p99_values.append(
            float(run["p99_ns"])
        )

    return {
        "runs": len(runs),
        "p99_mean_ns": statistics.mean(
            p99_values
        ),
        "p99_std_ns": statistics.stdev(
            p99_values
        ),
        "throughput_mean": statistics.mean(
            throughput_values
        ),
        "throughput_std": statistics.stdev(
            throughput_values
        ),
        "fairness_mean": (
            statistics.mean(fairness_values)
            if fairness_values
            else None
        ),
    }

def write_aggregate(groups):
    rows = []

    for workers, runs in groups.items():
        summary = summarize_group(runs)

        rows.append(
            {
                "workers": workers,
                "runs": summary["runs"],
                "p99_mean_ns": summary["p99_mean_ns"],
                "p99_std_ns": summary["p99_std_ns"],
                "throughput_mean": summary["throughput_mean"],
                "throughput_std": summary["throughput_std"],
                "fairness_mean": (
                    summary["fairness_mean"]
                    if summary["fairness_mean"] is not None
                    else "NA"
                ),  
            }
        )

        fieldnames = [
            "workers",
            "runs",
            "p99_mean_ns",
            "p99_std_ns",
            "throughput_mean",
            "throughput_std",
            "fairness_mean",
        ]

    with OUTPUT_PATH.open(
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
    rows = read_rows()
    groups = group_by_workers(rows)

    write_aggregate(groups)

    for workers, runs in groups.items():
        summary = summarize_group(runs)

        print(
            f"workers={workers} "
            f"runs={summary['runs']} "
            f"p99_mean_ns={summary['p99_mean_ns']:.3f} "
            f"p99_std_ns={summary['p99_std_ns']:.3f} "
            f"throughput_mean={summary['throughput_mean']:.3f} "
            f"throughput_std={summary['throughput_std']:.3f} "
            f"fairness_mean={summary['fairness_mean']}"
        )

if __name__ == "__main__":
    main()