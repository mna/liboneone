include_directories(
  "${PROJECT_SOURCE_DIR}"
  "${PROJECT_SOURCE_DIR}/deps"
)

add_executable(
  oneonetest

  channel.c
  locked_val.c
  main.c
  rwlocked_val.c
  spawn.c
  wait_group.c
  "${PROJECT_SOURCE_DIR}/deps/timer/src/timer.c"
)
target_link_libraries(oneonetest  oneone)
add_dependencies(oneonetest oneone)

