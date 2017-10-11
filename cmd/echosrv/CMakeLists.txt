include_directories(
  "${PROJECT_SOURCE_DIR}"
  "${PROJECT_SOURCE_DIR}/deps"
)

# source files for echosrv target.
set(
  echosrv_cfiles

  # NOTE: do not automate listing via a file(GLOB ...) command, as
  # this breaks cmake's ability to detect changes.
  main.c
)

# source files for the deps/socket99 third-party dependency
set(
  socket99_cfiles

  "${PROJECT_SOURCE_DIR}/deps/socket99/socket99.c"
)

add_executable(
  echosrv

  ${echosrv_cfiles}
  ${socket99_cfiles}
)
target_link_libraries(echosrv oneone)

