.RECIPEPREFIX := >

CC ?= gcc

CFLAGS = -O2 -Wall -Wextra -Werror -std=c11

BUILD_DIR = build

INTERACTIVE_SRC = workloads/interactive_latency.c
INTERACTIVE_BIN = $(BUILD_DIR)/interactive_latency

.PHONY: all clean env-check interactive-smoke test status

all: $(INTERACTIVE_BIN)

$(BUILD_DIR):
>mkdir -p $(BUILD_DIR)

$(INTERACTIVE_BIN): $(INTERACTIVE_SRC) | $(BUILD_DIR)
>$(CC) $(CFLAGS) $(INTERACTIVE_SRC) -o $(INTERACTIVE_BIN)

env-check:
>./scripts/check_environment.sh

interactive-smoke: $(INTERACTIVE_BIN)
>./scripts/smoke_interactive.sh $(INTERACTIVE_BIN)

test: env-check interactive-smoke

clean:
>rm -rf $(BUILD_DIR)

status:
>@echo "kernel=$$(uname -r)"
>@echo "sched_ext=$$(cat /sys/kernel/sched_ext/state 2>/dev/null || echo unavailable)"
