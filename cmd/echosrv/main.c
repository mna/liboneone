#include <stdio.h>
#include "oneone.h"
#include "attributes.h"

int
main(unused int argc, unused char ** argv) {
  one_version_s v = one_version();
  printf("%d.%d.%d\n", v.major, v.minor, v.patch);
}
