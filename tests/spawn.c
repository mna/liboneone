#include <unistd.h>

#include "../src/oneone.h"
#include "../deps/greatest/greatest.h"

static void
spawn_change_val(void * arg) {
  one_locked_val_s * lv = arg;
  int * val = one_locked_val_get(lv);
  *val = 10;
}

TEST
test_spawn() {
  int val = 1;
  one_locked_val_s * lv = one_locked_val_new(&val);
  one_spawn(spawn_change_val, lv);

  // TODO: better assert...
  usleep(1000);
  ASSERT_EQ(10, val);

  PASS();
}

SUITE(spawn) {
  RUN_TEST(test_spawn);
}

