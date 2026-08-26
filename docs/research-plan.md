# SchedLab Research Plan

## Research Question

How does a latency-aware sched_ext scheduling policy affect interactive
p99 scheduling latency under CPU contention compared with the Linux fair
scheduler, and what throughput and fairness trade-offs result?

## Compared Policies

### Linux Fair Scheduler

The normal Linux scheduler is the primary baseline.

### scx_simple

The Linux kernel `scx_simple` scheduler is the sched_ext reference baseline.

### SchedLab

SchedLab will implement a custom latency-aware sched_ext scheduler.

## Hypotheses

H1: SchedLab will reduce interactive p99 scheduling latency under CPU contention.

H2: Aggressive latency prioritization may reduce CPU-bound throughput.

H3: The latency-throughput trade-off will become larger as contention increases.

H4: Scheduling policy will affect fairness.

## Workloads

Interactive workers:

~~~text
sleep
wake
short CPU burst
sleep
~~~

CPU-bound workers continuously execute computation.

## Primary Metric

~~~text
interactive p99 scheduling latency
~~~

Additional measurements:

~~~text
p50 latency
p95 latency
maximum latency
CPU-bound throughput
context switches
CPU migrations
CPU utilization
fairness
~~~

## Research Phases

~~~text
1. environment validation
2. baseline latency workload
3. sched_ext instrumentation
4. custom scheduler implementation
5. correctness and starvation testing
6. experiment automation
7. frozen experiment matrix
8. replicated experiment
9. statistical analysis
10. technical report
~~~

## Completion Criteria

The project is complete when it contains:

~~~text
working custom scheduler
reproducible workloads
validated measurements
frozen experiment protocol
raw dataset
statistical analysis
figures
technical report
documented limitations
~~~
