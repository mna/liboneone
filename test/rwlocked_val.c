#include "../deps/greatest/greatest.h"
#include "../src/attributes.h"
#include "../src/errors.h"
#include "../src/oneone.h"

TEST test_rwlocked_val_new() {
  one_rwlocked_val_s* rwlv = one_rwlocked_val_new(NULL);
  ASSERT(rwlv);
  one_rwlocked_val_free(rwlv);

  PASS();
}

TEST test_rwlocked_val_free_returns_value() {
  int value = 42;
  one_rwlocked_val_s* rwlv = one_rwlocked_val_new(&value);
  int* free_val = one_rwlocked_val_free(rwlv);
  ASSERT_EQ(value, *free_val);

  PASS();
}

TEST test_rwlocked_val_get() {
  int value = 42;
  one_rwlocked_val_s* rwlv = one_rwlocked_val_new(&value);
  int* got_val = one_rwlocked_val_get(rwlv);
  ASSERT_EQ(value, *got_val);

  one_rwlocked_val_free(rwlv);

  PASS();
}

TEST test_rwlocked_val_set() {
  int val1 = 42;
  int val2 = 84;

  one_rwlocked_val_s* rwlv = one_rwlocked_val_new(&val1);
  int* old_val = one_rwlocked_val_set(rwlv, &val2);
  ASSERT_EQ(val1, *old_val);
  int* free_val = one_rwlocked_val_free(rwlv);
  ASSERT_EQ(val2, *free_val);

  PASS();
}

static void* with_rwlocked_val(unused void* val, unused void* const arg) {
  int* new_val = malloc(sizeof(*new_val));
  NULLFATAL(new_val, "out of memory");

  *new_val = 100;
  return new_val;
}

TEST test_rwlocked_val_with() {
  char* initial_value = "init";
  one_rwlocked_val_s* rwlv = one_rwlocked_val_new(initial_value);
  int* new_val = one_rwlocked_val_with(rwlv, with_rwlocked_val, NULL);
  ASSERT_EQ(100, *new_val);

  one_rwlocked_val_free(rwlv);
  free(new_val);

  PASS();
}

static void with_rdlocked_val(unused void* val, unused void* const arg) {}

TEST test_rwlocked_val_read_with() {
  int initial_value = 1;
  one_rwlocked_val_s* rwlv = one_rwlocked_val_new(&initial_value);
  int* val = one_rwlocked_val_read_with(rwlv, with_rdlocked_val, NULL);
  ASSERT_EQ(initial_value, *val);

  one_rwlocked_val_free(rwlv);

  PASS();
}

SUITE(rwlocked_val) {
  RUN_TEST(test_rwlocked_val_new);
  RUN_TEST(test_rwlocked_val_free_returns_value);
  RUN_TEST(test_rwlocked_val_get);
  RUN_TEST(test_rwlocked_val_set);
  RUN_TEST(test_rwlocked_val_with);
  RUN_TEST(test_rwlocked_val_read_with);
}
