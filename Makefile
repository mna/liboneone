# compiler and linker flags
CC ?= cc
CFLAGS ?= -Wall -Wextra -Wpedantic -Werror -g -std=c11
LDLIBS ?=
LDFLAGS ?=
EXTRACFLAGS := -Wformat=2										\
							 -Wshadow											\
							 -Wredundant-decls						\
							 -Wnested-externs							\
							 -Wmissing-include-dirs

# pathetic and lazy attempt to detect if the compiler is GCC
GCCVERSION := $(shell $(CC) --version | grep ^gcc | sed 's/^.* //g')
# See https://kristerw.blogspot.ca/2017/09/useful-gcc-warning-options-not-enabled.html
GCCEXTRA_WARNINGS := -Wduplicated-cond			\
										 -Wduplicated-branches	\
										 -Wlogical-op						\
										 -Wrestrict							\
										 -Wnull-dereference			\
										 -Wdouble-promotion

ifneq ($(GCCVERSION),)
	CFLAGS += $(GCCEXTRA_WARNINGS)
endif

# TODO: try on Linux, doesn't work on macOS
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
DEPS_DIR := deps

# list of source files for the project and dependencies
ONEONE_CFILES := src/spawn.c src/locked_val.c src/rwlocked_val.c src/wait_group.c src/channel.c
ONEONE_HFILES := src/oneone.h src/errors.h src/attributes.h
TIMER_CFILES := $(DEPS_DIR)/timer/src/timer.c
TEST_HFILES := $(ONEONE_HFILES) tests/suites.h
TEST_CFILES := $(ONEONE_CFILES) $(TIMER_CFILES) tests/main.c tests/spawn.c tests/locked_val.c tests/rwlocked_val.c tests/wait_group.c tests/channel.c

# default make target if none is specified.
.DEFAULT_GOAL := list

# list: list all available targets. See http://stackoverflow.com/questions/4219255/how-do-you-get-the-list-of-targets-in-a-makefile.
.PHONY: list
list:
	@$(MAKE) -pRrq -f $(lastword $(MAKEFILE_LIST)) : 2>/dev/null | awk -v RS= -F: '/^# File/,/^# Finished Make data base/ {if ($$1 !~ "^[#.]") {print $$1}}' | sort | egrep -v -e '^[^[:alnum:]]' -e '^$@$$'

# build test executable
$(BUILD_DIR)/test: $(TEST_CFILES) $(TEST_HFILES)
	$(CC) $(CFLAGS) $(EXTRACFLAGS) $(LDFLAGS) $(TEST_CFILES) $(LDLIBS) -o $@

# build and run test executable
.PHONY: test
test: $(BUILD_DIR)/test
	$^ -v | $(DEPS_DIR)/greatest/contrib/greenest

# list todos, fixmes and bugs
.PHONY: todo
todo:
	ag --ignore $(BUILD_DIR) --ignore $(DEPS_DIR) "\b(TODO|BUG|FIXME)\b"

# cleanup build artefacts
.PHONY: clean
clean:
	-rm -rf build/*

