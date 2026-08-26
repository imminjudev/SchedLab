#!/usr/bin/env bash
set -euo pipefail

binary="${1:-build/interactive_latency}"

tmp_file="$(mktemp)"

cleanup() {
    rm -f "$tmp_file"
}

trap cleanup EXIT

"$binary" \
    --iterations 20 \
    --warmup 5 \
    --period-us 2000 \
    --burst-us 100 \
    > "$tmp_file"

python3 - "$tmp_file" <<'PY'
import csv
import sys

path = sys.argv[1]

with open(
    path,
    newline="",
    encoding="utf-8",
) as handle:
    rows = list(
        csv.DictReader(handle)
    )

if len(rows) != 20:
    raise SystemExit(
        f"[FAIL] expected 20 samples, got {len(rows)}"
    )

required = {
    "sample",
    "target_ns",
    "actual_ns",
    "latency_ns",
    "cpu",
}

if set(rows[0]) != required:
    raise SystemExit(
        f"[FAIL] unexpected columns: {rows[0].keys()}"
    )

for index, row in enumerate(rows):
    if int(row["sample"]) != index:
        raise SystemExit(
            "[FAIL] sample sequence mismatch"
        )

    target = int(row["target_ns"])
    actual = int(row["actual_ns"])
    latency = int(row["latency_ns"])
    cpu = int(row["cpu"])

    if actual < target:
        raise SystemExit(
            "[FAIL] actual timestamp precedes target"
        )

    if latency != actual - target:
        raise SystemExit(
            "[FAIL] latency accounting mismatch"
        )

    if cpu < 0:
        raise SystemExit(
            "[FAIL] invalid CPU id"
        )

print(
    "[OK] interactive latency workload smoke test passed"
)
PY

python3 scripts/summarize_latency.py "$tmp_file"
