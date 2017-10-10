cmake_minimum_required(VERSION 3.9)
project(oneone VERSION 0.1.0 LANGUAGES C)

configure_file(
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
)
include_directories("${PROJECT_BINARY_DIR}")

option(BUILD_SHARED_LIBS "build as a shared library" OFF)

add_library(
  oneone

  channel.c
  locked_val.c
  rwlocked_val.c
  spawn.c
  wait_group.c
)

add_subdirectory(tests)

install(TARGETS oneone DESTINATION bin)
install(FILES oneone.h DESTINATION include)

