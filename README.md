# SchedLab

SchedLab is a Linux scheduling research project based on `sched_ext`.

## Research Question

How does a latency-aware sched_ext scheduling policy affect interactive
p99 scheduling latency under CPU contention compared with the Linux fair
scheduler, and what throughput and fairness trade-offs result?

## Policies

~~~text
Linux fair scheduler
scx_simple
SchedLab latency-aware scheduler
~~~

## Measurements

~~~text
p50 scheduling latency
p95 scheduling latency
p99 scheduling latency
maximum scheduling latency
CPU-bound throughput
context switches
CPU migrations
CPU utilization
fairness
~~~

## Development Environment

~~~text
WSL2
Ubuntu 26.04 LTS
Linux 6.18.40.1-microsoft-standard-WSL2+
CONFIG_SCHED_CLASS_EXT=y
clang 21.1.8
bpftool 7.7.0
pahole 1.31
~~~

WSL is used for implementation and functional validation.

Native Linux is preferred for the final accepted latency dataset.

## Project Structure

~~~text
scheduler/   sched_ext schedulers
workloads/   benchmark workloads
scripts/     experiment and analysis scripts
docs/        research documentation
results/     generated datasets
kernel/      local WSL kernel image, ignored by Git
~~~
