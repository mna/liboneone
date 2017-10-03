#include <stdio.h>
#include <unistd.h>
#include "../src/parallel.h"
#include "../deps/greatest/greatest.h"

static void _fnf(void *arg) {
  parallel_locked_val_s *lv = arg;
  int *val = parallel_locked_val_get(lv);
  *val = 10;
}

TEST test_fnf() {
  int val = 1;
  parallel_locked_val_s *lv = parallel_locked_val_new(&val);
  parallel_fnf(_fnf, lv);

  usleep(1000);
  ASSERT_EQ(10, val);

  PASS();
}

SUITE(fnf) {
  RUN_TEST(test_fnf);
}

