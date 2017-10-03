#include <stdio.h>
#include "../src/parallel.h"

#include "../deps/greatest/greatest.h"
#include "suites.h"

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();

  RUN_SUITE(fnf);

  GREATEST_MAIN_END();
}

