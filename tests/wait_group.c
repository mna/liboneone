#include <time.h>

#include "../src/parallel.h"
#include "../deps/greatest/greatest.h"

/*
static void _fnf(void *arg) {
  parallel_locked_val_s *lv = arg;
  int *val = parallel_locked_val_get(lv);
  *val = 10;
}
*/

TEST test_wait_group_wait_fnf() {
  parallel_wait_group_s *wg = parallel_wait_group_new(0);
  parallel_wait_group_free(wg);
  FAIL();
}

TEST test_wait_group_done_then_wait() {
  parallel_wait_group_s *wg = parallel_wait_group_new(1);
  parallel_wait_group_done(wg);

  clock_t start = clock();
  parallel_wait_group_wait(wg);
  clock_t end = clock();
  // TODO: assert on something else, too dependent on CPU
  ASSERT_IN_RANGE(0, end-start, 5);

  parallel_wait_group_free(wg);

  PASS();
}

TEST test_wait_group_new() {
  parallel_wait_group_s *wg = parallel_wait_group_new(1);
  ASSERT(wg);
  parallel_wait_group_free(wg);

  PASS();
}

SUITE(wait_group) {
  RUN_TEST(test_wait_group_new);
  RUN_TEST(test_wait_group_done_then_wait);
  RUN_TEST(test_wait_group_wait_fnf);
}

