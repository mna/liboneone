#include "../src/parallel.h"
#include "../src/_errors.h"
#include "../deps/greatest/greatest.h"

TEST test_rwlocked_val_new() {
  parallel_rwlocked_val_s *rwlv = parallel_rwlocked_val_new(NULL);
  ASSERT(rwlv);
  parallel_rwlocked_val_free(rwlv);

  PASS();
}

TEST test_rwlocked_val_free_returns_value() {
  int value = 42;
  parallel_rwlocked_val_s *rwlv = parallel_rwlocked_val_new(&value);
  int *free_val = parallel_rwlocked_val_free(rwlv);
  ASSERT_EQ(value, *free_val);

  PASS();
}

TEST test_rwlocked_val_get() {
  int value = 42;
  parallel_rwlocked_val_s *rwlv = parallel_rwlocked_val_new(&value);
  int *got_val = parallel_rwlocked_val_get(rwlv);
  ASSERT_EQ(value, *got_val);

  parallel_rwlocked_val_free(rwlv);

  PASS();
}

TEST test_rwlocked_val_set() {
  int val1 = 42;
  int val2 = 84;

  parallel_rwlocked_val_s *rwlv = parallel_rwlocked_val_new(&val1);
  int *old_val = parallel_rwlocked_val_set(rwlv, &val2);
  ASSERT_EQ(val1, *old_val);
  int *free_val = parallel_rwlocked_val_free(rwlv);
  ASSERT_EQ(val2, *free_val);

  PASS();
}

static void * _with_rwlocked_val(void *val) {
  int *new_val = malloc(sizeof(int));
  NULLFATAL(new_val, "out of memory");

  *new_val = 100;
  return new_val;
}

TEST test_rwlocked_val_wrwith() {
  char *initial_value = "init";
  parallel_rwlocked_val_s *rwlv = parallel_rwlocked_val_new(initial_value);
  int *new_val = parallel_rwlocked_val_wrwith(rwlv, _with_rwlocked_val);
  ASSERT_EQ(100, *new_val);

  parallel_rwlocked_val_free(rwlv);
  free(new_val);

  PASS();
}

static void _with_rdlocked_val(void *val) {
}

TEST test_rwlocked_val_rdwith() {
  int initial_value = 1;
  parallel_rwlocked_val_s *rwlv = parallel_rwlocked_val_new(&initial_value);
  int *val = parallel_rwlocked_val_rdwith(rwlv, _with_rdlocked_val);
  ASSERT_EQ(initial_value, *val);

  parallel_rwlocked_val_free(rwlv);

  PASS();
}

SUITE(rwlocked_val) {
  RUN_TEST(test_rwlocked_val_new);
  RUN_TEST(test_rwlocked_val_free_returns_value);
  RUN_TEST(test_rwlocked_val_get);
  RUN_TEST(test_rwlocked_val_set);
  RUN_TEST(test_rwlocked_val_wrwith);
  RUN_TEST(test_rwlocked_val_rdwith);
}

