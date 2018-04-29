# Common Makefile options:
# ------------------------
#
# RELEASE=1 : optimized build (default: debug build)
# TAP=1     : tap-based test output (default: greenest)
# VERBOSE=1 : verbose make output (default: quiet)
#
# Common Makefile targets:
# ------------------------
#
# make test : build and run tests
# make run : build and run binary
# make clean : clean build files
# make test-mem : run tests with memory leak checks (should not run with RELEASE=1)
# make test-race : run tests with thread checks (should not run with RELEASE=1)
#

# TODO: build shared/static library
# TODO: defer make targets to build cmd/*
# TODO: manage object files

# Enable verbose builds with VERBOSE=1.
V := @
ifeq ($(VERBOSE),1)
	V :=
endif

# CC defaults to cc anyway, but it's nicer to have it explicit.
CC ?= cc

# some compiler-specific flags may need to be set, so determine which
# compiler is used among the major ones (will be set to 1 for true, 0
# for false).
CCISGCC   := $(shell $(CC) --version | grep --ignore-case gcc --count)
CCISCLANG := $(shell $(CC) --version | grep --ignore-case clang --count)

# CFLAGS should set the pkg-config cflags for the shared deps if required.
CFLAGS := -std=c11 \
					-D_POSIX_C_SOURCE=200809L \
					-D_XOPEN_SOURCE=700 \
					-Wall -Wpedantic -Wextra -Werror -g -Wformat=2 -Wshadow \
					-Wredundant-decls -Wnested-externs -Wmissing-include-dirs
ifeq ($(CCISGCC), 1)
	# gcc-specific flags
	CFLAGS += -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wrestrict \
						-Wnull-dereference -Wdouble-promotion
endif

# LDLIBS lists the libraries required for the build.
LDLIBS := -lpthread
# LDFLAGS lists the non-library flags to pass to the linker.
LDFLAGS :=

# Set the optimization level depending on build mode.
ifeq ($(RELEASE), 1)
	CFLAGS += -O3
else
	CFLAGS += -O0
endif

# list of source files for the project
SRC_CFILES := $(wildcard src/*.c)
SRC_HFILES := $(wildcard src/*.h)
TIMER_CFILES := deps/timer/src/timer.c
TIMER_HFILES := deps/timer/src/timer.h
TEST_CFILES := $(SRC_CFILES) $(TIMER_CFILES) $(wildcard test/*.c)
TEST_HFILES := $(SRC_HFILES) $(TIMER_HFILES) $(wildcard test/*.h)

# build information
BUILD_DIR := build
TEST_BINARY := test

# test output can be switched to tap with TAP=1.
TEST_OUTPUT := deps/greatest/contrib/greenest
ifeq ($(TAP), 1)
	TEST_OUTPUT := deps/greatest/contrib/entapment
endif

# default make target if none is specified is to list
# the targets.
.DEFAULT_GOAL := list

# create build directory
$(BUILD_DIR):
	$(V) mkdir -p $@

# build test executable
$(BUILD_DIR)/$(TEST_BINARY): $(TEST_CFILES) $(TEST_HFILES) | $(BUILD_DIR)
	$(V) $(CC) $(CFLAGS) $(TEST_CFILES) $(LDFLAGS) $(LDLIBS) -o $@

# run test executable
.PHONY: test
test: $(BUILD_DIR)/$(TEST_BINARY)
	$(V) $^ -v | $(TEST_OUTPUT)

# run memory checks on test binary
.PHONY: test-mem
test-mem: $(BUILD_DIR)/$(TEST_BINARY)
	$(V) valgrind --leak-check=yes $^ -v

# run thread checks on test binary
.PHONY: test-race
test-race: $(BUILD_DIR)/$(TEST_BINARY)
	$(V) valgrind --tool=helgrind $^ -v

# cleanup build artefacts
.PHONY: clean
clean:
	$(V) -rm -f $(BUILD_DIR)/*

# list: list all available targets.
# See http://stackoverflow.com/questions/4219255/how-do-you-get-the-list-of-targets-in-a-makefile.
.PHONY: list
list:
	@$(MAKE) -pRrq -f $(lastword $(MAKEFILE_LIST)) : 2>/dev/null | awk -v RS= -F: '/^# File/,/^# Finished Make data base/ {if ($$1 !~ "^[#.]") {print $$1}}' | sort | egrep -v -e '^[^[:alnum:]]' -e '^$@$$'

# print all makefile variables, see https://www.cmcrossroads.com/article/dumping-every-makefile-variable
.PHONY: printvars
printvars:
	@$(foreach v,$(sort $(.VARIABLES)), \
		 $(if $(filter-out environment% default automatic, \
		 $(origin $v)),$(warning $v=$($v) ($(value $v)))))

# help! remind a little bit how important tasks are done.
.PHONY: help
help:
	@echo "Add all required .c and .h files to the SRC_CFILES and SRC_HFILES variables."
	@echo
	@echo "    make test"
	@echo "        builds and runs the tests."
	@echo
	@echo "    git submodule add [-b branch_name] <git url> deps/<dep_name>"
	@echo "        adds a new dependency."
	@echo
	@echo "    git rm deps/<dep_name>"
	@echo "    rm -rf .git/modules/deps/<dep_name>"
	@echo "        removes a dependency."
	@echo
	@echo "    git submodule update --init"
	@echo "        installs and fetches the registered submodules (on a fresh clone)."
	@echo

