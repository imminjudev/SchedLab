#!/usr/bin/env bash
set -euo pipefail

output_dir="build/contention-smoke"

./scripts/run_contention_case.sh \
    1 \
    "$output_dir" \
    0 \
    50 \
    5 \
    2000 \
    100

python3 - "$output_dir" <<'PY'
import csv
import sys
from pathlib import Path

root = Path(sys.argv[1])

latency_path = root / "latency.csv"
worker_path = root / "cpu_workers.csv"

with latency_path.open(
    newline="",
    encoding="utf-8",
) as handle:
    latency_rows = list(csv.DictReader(handle))

if len(latency_rows) != 50:
    raise SystemExit(
        "[FAIL] unexpected latency sample count"
    )

with worker_path.open(
    newline="",
    encoding="utf-8",
) as handle:
    worker_rows = list(csv.DictReader(handle))

if len(worker_rows) != 1:
    raise SystemExit(
        "[FAIL] unexpected worker count"
    )

if int(worker_rows[0]["work_units"]) <= 0:
    raise SystemExit(
        "[FAIL] CPU worker made no progress"
    )

metadata = (
    root / "metadata.txt"
).read_text(
    encoding="utf-8"
)

required = [
    "sched_ext_state=disabled",
    "cpu=0",
    "workers=1",
]

for item in required:
    if item not in metadata:
        raise SystemExit(
            f"[FAIL] missing metadata: {item}"
        )

print(
    "[OK] CPU contention smoke test passed"
)
PY
