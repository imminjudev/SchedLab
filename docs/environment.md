# SchedLab Development Environment

## CPU

~~~text
AMD Ryzen 5 5600
6 physical cores
12 hardware threads
1 NUMA node
~~~

## Linux

~~~text
Ubuntu 26.04 LTS
WSL2
Linux 6.18.40.1-microsoft-standard-WSL2+
~~~

## Custom Kernel

Source:

~~~text
microsoft/WSL2-Linux-Kernel
branch: linux-msft-wsl-6.18.y
source commit:
14794180686c2fb6307fbe359c359bec765249f3
~~~

Verified configuration:

~~~text
CONFIG_BPF=y
CONFIG_BPF_SYSCALL=y
CONFIG_BPF_JIT=y
CONFIG_BPF_JIT_ALWAYS_ON=y
CONFIG_BPF_JIT_DEFAULT_ON=y
CONFIG_SCHED_CLASS_EXT=y
CONFIG_DEBUG_INFO_BTF=y
~~~

## Toolchain

~~~text
clang 21.1.8
bpftool 7.7.0
libbpf 1.7
pahole 1.31
GNU Make 4.4.1
Git 2.53.0
~~~

## sched_ext Validation

The kernel-provided `scx_simple` scheduler was successfully built,
attached, executed, and detached.

Before execution:

~~~text
state=disabled
nr_rejected=0
~~~

During execution, local and global dispatch counters increased.

After Ctrl+C:

~~~text
EXIT: unregistered from user space
state=disabled
nr_rejected=0
~~~
