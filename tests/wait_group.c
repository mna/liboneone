#include <unistd.h>

#include "../src/parallel.h"
#include "../deps/timer/src/timer.h"
#include "../deps/greatest/greatest.h"

long sleep_before_done = 1000;

static void _spawn(void *arg) {
  long *us = arg;
  usleep(*us);
}

static void _spawn_wait(void *arg) {
  parallel_wait_group_s *wg = arg;
  parallel_wait_group_wait(wg);
}

TEST test_wait_group_multi_wait() {
  parallel_wait_group_s *wg1 = parallel_wait_group_new(0);
  parallel_wait_group_s *wg2 = parallel_wait_group_new(1);

  // start 2 spawn waiting on wg2, waited for by wg1
  parallel_spawn_wg(wg1, _spawn_wait, wg2);
  parallel_spawn_wg(wg1, _spawn_wait, wg2);

  usleep(sleep_before_done);

  timer_t timer; timer_start(&timer);
  parallel_wait_group_done(wg2);
  parallel_wait_group_wait(wg1);
  long us = (long)timer_delta_us(&timer);
  ASSERT_IN_RANGE(0, us, 100);

  parallel_wait_group_free(wg1);
  parallel_wait_group_free(wg2);

  PASS();
}

TEST test_wait_group_wait_spawn() {
  parallel_wait_group_s *wg = parallel_wait_group_new(0);

  timer_t timer; timer_start(&timer);

  parallel_spawn_wg(wg, _spawn, &sleep_before_done);
  parallel_spawn_wg(wg, _spawn, &sleep_before_done);
  parallel_wait_group_wait(wg);

  long us = (long)timer_delta_us(&timer);
  ASSERT_IN_RANGE(sleep_before_done, us, sleep_before_done);

  parallel_wait_group_free(wg);

  PASS();
}

TEST test_wait_group_done_then_wait() {
  parallel_wait_group_s *wg = parallel_wait_group_new(1);
  parallel_wait_group_done(wg);

  timer_t timer; timer_start(&timer);
  parallel_wait_group_wait(wg);
  long us = (long)timer_delta_us(&timer);
  ASSERT_IN_RANGE(0L, us, 5L);

  parallel_wait_group_free(wg);

  PASS();
}

TEST test_wait_group_new() {
  parallel_wait_group_s *wg = parallel_wait_group_new(1);
  ASSERT(wg);
  parallel_wait_group_free(wg);

  PASS();
}

TEST test_wait_group_wait_on_zero() {
  parallel_wait_group_s *wg = parallel_wait_group_new(0);

  timer_t timer; timer_start(&timer);
  parallel_wait_group_wait(wg);
  long us = (long)timer_delta_us(&timer);
  ASSERT_IN_RANGE(0L, us, 5L);

  parallel_wait_group_free(wg);
  PASS();
}

SUITE(wait_group) {
  RUN_TEST(test_wait_group_new);
  RUN_TEST(test_wait_group_done_then_wait);
  RUN_TEST(test_wait_group_wait_spawn);
  RUN_TEST(test_wait_group_multi_wait);
  RUN_TEST(test_wait_group_wait_on_zero);
}

