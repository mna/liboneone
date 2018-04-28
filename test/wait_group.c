#define timer_t time_timer_t
#include <time.h>
#undef timer_t

#include "../deps/greatest/greatest.h"
#include "../deps/timer/src/timer.h"
#include "../src/oneone.h"

static struct timespec sleep_duration = {.tv_nsec = 1000000};

static void spawn(void* arg) {
  long* us = arg;
  struct timespec ts = {.tv_nsec = *us * 1000};
  nanosleep(&ts, NULL);
}

static void spawn_wait(void* arg) {
  one_wait_group_s* wg = arg;
  one_wait_group_wait(wg);
}

TEST test_wait_group_multi_wait() {
  one_wait_group_s* wg1 = one_wait_group_new(0);
  one_wait_group_s* wg2 = one_wait_group_new(1);

  // start 2 spawn waiting on wg2, waited for by wg1
  one_spawn_wg(wg1, spawn_wait, wg2);
  one_spawn_wg(wg1, spawn_wait, wg2);

  nanosleep(&sleep_duration, NULL);

  timer_t timer;
  timer_start(&timer);
  one_wait_group_done(wg2);
  one_wait_group_wait(wg1);
  long us = (long)timer_delta_us(&timer);
  ASSERT_IN_RANGE(0, us, 100);

  one_wait_group_free(wg1);
  one_wait_group_free(wg2);

  PASS();
}

TEST test_wait_group_wait_spawn() {
  one_wait_group_s* wg = one_wait_group_new(0);

  timer_t timer;
  timer_start(&timer);

  one_spawn_wg(wg, spawn, &sleep_duration);
  one_spawn_wg(wg, spawn, &sleep_duration);
  one_wait_group_wait(wg);

  long us = (long)timer_delta_us(&timer);
  ASSERT_IN_RANGE(sleep_duration.tv_nsec / 1000, us,
                  sleep_duration.tv_nsec / 1000);

  one_wait_group_free(wg);

  PASS();
}

TEST test_wait_group_done_then_wait() {
  one_wait_group_s* wg = one_wait_group_new(1);
  one_wait_group_done(wg);

  timer_t timer;
  timer_start(&timer);
  one_wait_group_wait(wg);
  long us = (long)timer_delta_us(&timer);
  ASSERT_IN_RANGE(0L, us, 5L);

  one_wait_group_free(wg);

  PASS();
}

TEST test_wait_group_new() {
  one_wait_group_s* wg = one_wait_group_new(1);
  ASSERT(wg);
  one_wait_group_free(wg);

  PASS();
}

TEST test_wait_group_wait_on_zero() {
  one_wait_group_s* wg = one_wait_group_new(0);

  timer_t timer;
  timer_start(&timer);
  one_wait_group_wait(wg);
  long us = (long)timer_delta_us(&timer);
  ASSERT_IN_RANGE(0L, us, 5L);

  one_wait_group_free(wg);
  PASS();
}

SUITE(wait_group) {
  RUN_TEST(test_wait_group_new);
  RUN_TEST(test_wait_group_done_then_wait);
  RUN_TEST(test_wait_group_wait_spawn);
  RUN_TEST(test_wait_group_multi_wait);
  RUN_TEST(test_wait_group_wait_on_zero);
}
