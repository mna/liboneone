#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../src/parallel.h"
#include "../deps/greatest/greatest.h"

static void
_spawn(void *arg) {
  parallel_locked_val_s *lv = arg;
  int *val = parallel_locked_val_get(lv);
  *val = 10;
}

TEST
test_spawn() {
  int val = 1;
  parallel_locked_val_s *lv = parallel_locked_val_new(&val);
  parallel_spawn(_spawn, lv);

  usleep(1000);
  ASSERT_EQ(10, val);

  PASS();
}

SUITE(spawn) {
  RUN_TEST(test_spawn);
}

