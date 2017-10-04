# compiler and linker flags
CC ?= cc
CFLAGS ?= -Wall -pedantic -Werror -g -std=c11
LDLIBS ?=
LDFLAGS ?=

# ifeq ($(msan),1)
# 	# see https://clang.llvm.org/docs/MemorySanitizer.html
# 	CFLAGS += -fsanitize=memory -fno-omit-frame-pointer -O1
# 	LDLIBS += -lasan
# endif
# ifeq ($(tsan),1)
# 	CFLAGS += -fsanitize=thread
# 	LDLIBS += -lasan
# endif

# build in child directory for easier cleanup
BUILD_DIR := build

# list of source files for the project and dependencies
PARALLEL_CFILES := src/spawn.c src/locked_val.c src/wait_group.c
PARALLEL_HFILES := src/parallel.h src/_errors.h
TIMER_CFILES := deps/timer/src/timer.c
TEST_HFILES := $(PARALLEL_HFILES) tests/suites.h
TEST_CFILES := $(PARALLEL_CFILES) $(TIMER_CFILES) tests/main.c tests/spawn.c tests/locked_val.c tests/wait_group.c

# default make target if none is specified.
.DEFAULT_GOAL := list

# list: list all available targets. See http://stackoverflow.com/questions/4219255/how-do-you-get-the-list-of-targets-in-a-makefile.
.PHONY: list
list:
	@$(MAKE) -pRrq -f $(lastword $(MAKEFILE_LIST)) : 2>/dev/null | awk -v RS= -F: '/^# File/,/^# Finished Make data base/ {if ($$1 !~ "^[#.]") {print $$1}}' | sort | egrep -v -e '^[^[:alnum:]]' -e '^$@$$'

# build test executable
$(BUILD_DIR)/test: $(TEST_CFILES) $(TEST_HFILES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(TEST_CFILES) $(LDLIBS) -o $@

# build and run test executable
.PHONY: test
test: $(BUILD_DIR)/test
	$^ -v | deps/greatest/contrib/greenest

# cleanup build artefacts
.PHONY: clean
clean:
	-rm -rf build/*

