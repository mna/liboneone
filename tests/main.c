#include <stdlib.h>

#include "../src/parallel.h"

#include "../deps/greatest/greatest.h"
#include "suites.h"

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();

  RUN_SUITE(spawn);
  RUN_SUITE(locked_val);
  RUN_SUITE(rwlocked_val);
  RUN_SUITE(wait_group);
  RUN_SUITE(channel);

  GREATEST_MAIN_END();
}

