#!/usr/bin/env bash
set -euo pipefail

workers="${1:-1}"
output_dir="${2:-build/contention-case}"
cpu="${3:-0}"
iterations="${4:-100}"
warmup="${5:-20}"
period_us="${6:-5000}"
burst_us="${7:-250}"

interactive_bin="build/interactive_latency"
cpu_bin="build/cpu_bound"

if [[ ! -x "$interactive_bin" ]]; then
    echo "[FAIL] missing $interactive_bin"
    exit 1
fi

if [[ ! -x "$cpu_bin" ]]; then
    echo "[FAIL] missing $cpu_bin"
    exit 1
fi

if [[ ! "$workers" =~ ^[0-9]+$ ]]; then
    echo "[FAIL] workers must be a non-negative integer"
    exit 1
fi

if [[ ! -r /sys/kernel/sched_ext/state ]]; then
    echo "[FAIL] sched_ext state unavailable"
    exit 1
fi

sched_ext_state="$(cat /sys/kernel/sched_ext/state)"

if [[ "$sched_ext_state" != "disabled" ]]; then
    echo "[FAIL] baseline run requires sched_ext=disabled"
    exit 1
fi

rm -rf "$output_dir"
mkdir -p "$output_dir/workers"

total_samples=$((iterations + warmup))
interactive_runtime_us=$((total_samples * period_us))
worker_duration_ms=$(((interactive_runtime_us + 999) / 1000 + 1200))

pids=()

cleanup() {
    if (( ${#pids[@]} > 0 )); then
        kill "${pids[@]}" 2>/dev/null || true
    fi
}

trap cleanup EXIT INT TERM

for ((worker = 0; worker < workers; worker++)); do
    "$cpu_bin" \
        --duration-ms "$worker_duration_ms" \
        --cpu "$cpu" \
        --worker-id "$worker" \
        > "$output_dir/workers/worker-$worker.csv" \
        2> "$output_dir/workers/worker-$worker.stderr" &

    pids+=("$!")
done

if (( workers > 0 )); then
    sleep 0.1
fi

"$interactive_bin" \
    --iterations "$iterations" \
    --warmup "$warmup" \
    --period-us "$period_us" \
    --burst-us "$burst_us" \
    --cpu "$cpu" \
    > "$output_dir/latency.csv" \
    2> "$output_dir/interactive.stderr"

if (( workers > 0 )); then
    for pid in "${pids[@]}"; do
        wait "$pid"
    done
fi

trap - EXIT INT TERM

echo "worker_id,cpu,duration_ns,work_units" > "$output_dir/cpu_workers.csv"

for ((worker = 0; worker < workers; worker++)); do
    tail -n 1 \
        "$output_dir/workers/worker-$worker.csv" \
        >> "$output_dir/cpu_workers.csv"
done

{
    echo "kernel=$(uname -r)"
    echo "sched_ext_state=$sched_ext_state"
    echo "cpu=$cpu"
    echo "workers=$workers"
    echo "iterations=$iterations"
    echo "warmup=$warmup"
    echo "period_us=$period_us"
    echo "burst_us=$burst_us"
    echo "worker_duration_ms=$worker_duration_ms"
} > "$output_dir/metadata.txt"

python3 scripts/summarize_latency.py \
    "$output_dir/latency.csv" \
    > "$output_dir/latency_summary.txt"

python3 scripts/summarize_throughput.py \
    "$output_dir/cpu_workers.csv" \
    > "$output_dir/throughput_summary.txt"

echo "=== latency ==="
cat "$output_dir/latency_summary.txt"

echo
echo "=== throughput ==="
cat "$output_dir/throughput_summary.txt"

echo
echo "[OK] contention case complete"
