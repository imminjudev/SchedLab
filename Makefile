.RECIPEPREFIX := >

.PHONY: env-check status

env-check:
>./scripts/check_environment.sh

status:
>@echo "kernel=$$(uname -r)"
>@echo "sched_ext=$$(cat /sys/kernel/sched_ext/state 2>/dev/null || echo unavailable)"
