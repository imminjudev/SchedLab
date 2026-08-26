#!/usr/bin/env bash
set -euo pipefail

echo "=== kernel ==="
uname -r

echo
echo "=== sched_ext ==="

if [[ ! -d /sys/kernel/sched_ext ]]; then
    echo "[FAIL] sched_ext unavailable"
    exit 1
fi

echo "state=$(cat /sys/kernel/sched_ext/state)"
echo "nr_rejected=$(cat /sys/kernel/sched_ext/nr_rejected)"

echo
echo "=== BTF ==="

if [[ ! -r /sys/kernel/btf/vmlinux ]]; then
    echo "[FAIL] BTF unavailable"
    exit 1
fi

ls -lh /sys/kernel/btf/vmlinux

echo
echo "=== toolchain ==="
clang --version | head -1
bpftool version
pahole --version
make --version | head -1
git --version

echo
echo "[OK] SchedLab development environment ready"
