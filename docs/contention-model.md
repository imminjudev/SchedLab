# CPU Contention Model

SchedLab creates controlled CPU contention by pinning the interactive workload
and one or more CPU-bound workers to the same logical CPU.

The initial topology is:

~~~text
logical CPU N

    interactive periodic task
            +
    CPU-bound worker 0
            +
    CPU-bound worker 1
            +
            ...
~~~

All participating tasks remain runnable on the same selected CPU when awake.

## CPU-Bound Work Unit

A CPU worker repeatedly executes a fixed block of integer mixing operations.

One completed block is recorded as one `work_unit`.

The absolute value of a work unit is implementation-specific.

It is intended for policy comparisons using the same binary and experiment
configuration.

## Throughput

Per-worker throughput is calculated as:

~~~text
work_units
/
measured worker duration
~~~

Aggregate CPU-bound throughput is the sum of all worker throughputs.

## Fairness

SchedLab initially reports Jain's fairness index across CPU-bound worker
throughputs.

~~~text
fairness = 1
~~~

indicates equal observed throughput among workers.

## Baseline Policy

The initial contention runner requires:

~~~text
/sys/kernel/sched_ext/state = disabled
~~~

This ensures that the collected case uses the normal Linux scheduling classes
rather than an attached sched_ext policy.

Later experiment infrastructure will explicitly select and record each
scheduler policy.

## Measurement Scope

The interactive workload continues to report `release_latency`.

The CPU worker provides background throughput.

These measurements allow SchedLab to study the trade-off between:

~~~text
interactive release latency
        vs
background CPU throughput
        vs
fairness
~~~

Kernel-level runnable-to-scheduled latency will be added in a later
instrumentation phase.
