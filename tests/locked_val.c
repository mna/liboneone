#include "oneone.h"
#include "errors.h"
#include "attributes.h"
#include "greatest/greatest.h"

TEST
test_locked_val_new() {
  one_locked_val_s * lv = one_locked_val_new(NULL);
  ASSERT(lv);
  one_locked_val_free(lv);

  PASS();
}

TEST
test_locked_val_free_returns_value() {
  int value = 42;
  one_locked_val_s * lv = one_locked_val_new(&value);
  int * free_val = one_locked_val_free(lv);
  ASSERT_EQ(value, *free_val);

  PASS();
}

TEST
test_locked_val_get() {
  int value = 42;
  one_locked_val_s * lv = one_locked_val_new(&value);
  int * got_val = one_locked_val_get(lv);
  ASSERT_EQ(value, *got_val);
  one_locked_val_free(lv);

  PASS();
}

TEST
test_locked_val_set() {
  int val1 = 42;
  int val2 = 84;

  one_locked_val_s * lv = one_locked_val_new(&val1);
  int * old_val = one_locked_val_set(lv, &val2);
  ASSERT_EQ(val1, *old_val);
  int * free_val = one_locked_val_free(lv);
  ASSERT_EQ(val2, *free_val);

  PASS();
}

static void *
with_locked_val(unused void * val, unused void * const arg) {
  int * new_val = malloc(sizeof(*new_val));
  NULLFATAL(new_val, "out of memory");

  *new_val = 100;
  return new_val;
}

TEST
test_locked_val_with() {
  char * initial_value = "init";
  one_locked_val_s * lv = one_locked_val_new(initial_value);
  int * new_val = one_locked_val_with(lv, with_locked_val, NULL);
  ASSERT_EQ(100, *new_val);

  one_locked_val_free(lv);
  free(new_val);

  PASS();
}

SUITE(locked_val) {
  RUN_TEST(test_locked_val_new);
  RUN_TEST(test_locked_val_free_returns_value);
  RUN_TEST(test_locked_val_get);
  RUN_TEST(test_locked_val_set);
  RUN_TEST(test_locked_val_with);
}

