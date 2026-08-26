# SchedLab Measurement Model

## Release Latency

The initial interactive workload uses an absolute periodic release time.

For each iteration:

~~~text
target release time
        |
        v
clock_nanosleep(TIMER_ABSTIME)
        |
        v
task becomes eligible to wake
        |
        v
Linux scheduling / wakeup path
        |
        v
userspace resumes
        |
        v
actual timestamp
~~~

The initial metric is:

~~~text
release_latency =
    actual_userspace_resume_time
    -
    target_release_time
~~~

This measurement includes more than runqueue waiting time.

It may include:

~~~text
timer expiry delay
kernel wakeup processing
runqueue waiting
scheduler decision delay
return to userspace
virtualization effects
~~~

SchedLab therefore calls this metric `release latency`.

It must not be reported as pure kernel scheduling latency.

## Kernel Scheduling Latency

A later instrumentation phase will measure a narrower interval using kernel
events associated with task wakeup and actual scheduling.

Conceptually:

~~~text
task becomes runnable
        |
        v
scheduler waiting interval
        |
        v
task is selected to run
~~~

That measurement will be used when making claims specifically about scheduling
latency.

## Absolute Releases

The workload uses `CLOCK_MONOTONIC` with `TIMER_ABSTIME`.

Absolute deadlines prevent accumulated sleep drift from changing the intended
period after every iteration.

If the task is already late when the next release time arrives, the lateness is
preserved in the measured release latency.

## CPU Burst

After each wakeup measurement, the interactive task performs a configurable
CPU burst.

The initial workload requires:

~~~text
burst_us < period_us
~~~

This creates a periodic sleep/wakeup workload whose CPU demand can later be
combined with CPU-bound background workers.

## CPU Tracking

Each sample records the CPU returned by `sched_getcpu()`.

Adjacent samples running on different CPUs are counted as observed migrations
by the summary script.

This is an application-level migration observation and does not replace later
kernel-level scheduling instrumentation.
