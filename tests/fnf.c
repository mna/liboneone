#include <stdio.h>
#include "../src/parallel.h"
#include "../deps/greatest/greatest.h"

static void _print(void *arg) {
  printf(">> inside fnf\n");
}

TEST test_session_fnf() {
  parallel_fnf(_print, NULL);
  PASS();
}

SUITE(fnf) {
  RUN_TEST(test_session_fnf);
}

