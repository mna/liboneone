cmake_minimum_required(VERSION 3.9)
project(oneone VERSION 0.1.0 LANGUAGES C)

# TODO: actually expose the version via a func
configure_file(
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
)
include_directories("${PROJECT_BINARY_DIR}")

add_compile_options(
  -Wall
  -Wextra
  -Wpedantic
  -Werror
  -g
  -std=c11
  -Wformat=2
  -Wshadow
  -Wredundant-decls
  -Wnested-externs
  -Wmissing-include-dirs
)

# additional flags for GCC compiler
if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
  add_compile_options(
    -Wduplicated-cond
    -Wduplicated-branches
    -Wlogical-op
    -Wrestrict
    -Wnull-dereference
    -Wdouble-promotion
  )
endif()

option(BUILD_SHARED_LIBS "build as a shared library" OFF)

# define the main oneone target
add_library(
  oneone

  channel.c
  locked_val.c
  rwlocked_val.c
  spawn.c
  wait_group.c
)

# add the tests
add_subdirectory(tests)

install(TARGETS oneone DESTINATION bin)
install(FILES oneone.h DESTINATION include)

