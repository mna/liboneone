include_directories(
  "${PROJECT_SOURCE_DIR}"
  "${PROJECT_SOURCE_DIR}/deps"
)

# source files for oneonetest target.
set(
  oneonetest_cfiles

  # NOTE: do not automate listing via a file(GLOB ...) command, as
  # this breaks cmake's ability to detect changes.
  channel.c
  locked_val.c
  main.c
  rwlocked_val.c
  spawn.c
  wait_group.c
)

# source files for the deps/timer third-party dependency
set(
  timerdep_cfiles

  "${PROJECT_SOURCE_DIR}/deps/timer/src/timer.c"
)

add_executable(
  oneonetest

  ${oneonetest_cfiles}
  ${timerdep_cfiles}
)
target_link_libraries(oneonetest oneone)
add_dependencies(oneonetest oneone)

