#include <pthread.h>
#include <unistd.h>

#include "oneone.h"
#include "errors.h"
#include "config.h"

one_version_s
one_version() {
  return (one_version_s) {
    .major = oneone_VERSION_MAJOR,
    .minor = oneone_VERSION_MINOR,
    .patch = oneone_VERSION_PATCH,
  };
}

static size_t
get_default_stack_size() {
  static size_t default_stack_size;
  if(default_stack_size) {
    return default_stack_size;
  }

  size_t page_size = 1024 * 8; // default page size to 8KB
  long sz = sysconf(_SC_PAGESIZE);
  if(sz) {
    page_size = (size_t)sz;
  }

  // default stack size to 4 pages
  default_stack_size = 4 * page_size;
  return default_stack_size;
}

typedef struct spawn_s {
  void (*fn) (void *);
  void * arg;
  one_wait_group_s *wg;
} spawn_s;

static void *
spawn_thunk(void * arg) {
  spawn_s *spawn = arg;
  if(spawn) {
    one_wait_group_s *wg = spawn->wg;
    spawn->fn(spawn->arg);
    free(spawn);

    if(wg) {
      one_wait_group_done(wg);
    }
  }
  return NULL;
}

int
one_spawn(void (*fn) (void *), void * arg) {
  return one_spawn_wg_ssz(NULL, fn, arg, get_default_stack_size());
}

int
one_spawn_wg(one_wait_group_s * const wg, void (*fn) (void *), void * arg) {
  return one_spawn_wg_ssz(wg, fn, arg, get_default_stack_size());
}

int
one_spawn_ssz(void (*fn) (void *), void * arg, size_t stack_sz) {
  return one_spawn_wg_ssz(NULL, fn, arg, stack_sz);
}

int
one_spawn_wg_ssz(one_wait_group_s * const wg, void (*fn) (void *), void * arg, size_t stack_sz) {
  pthread_attr_t attr;
  pthread_t t;
  int err = 0;
  spawn_s * spawn = NULL;

  // configure the thread's stack size
  err = pthread_attr_init(&attr);
  ERRCLEANUP(err, 0);
  err = pthread_attr_setstacksize(&attr, stack_sz);
  ERRCLEANUP(err, 1);

  // create the thunk to call fn with the signature expected by pthread
  spawn = malloc(sizeof(*spawn));
  NULLFATAL(spawn, "out of memory");
  spawn->fn = fn;
  spawn->arg = arg;
  spawn->wg = wg;

  // at this point the wait group must be incremented
  if(wg) {
    one_wait_group_add(wg, 1);
  }

  // create the detached thread
  err = pthread_create(&t, &attr, spawn_thunk, spawn);
  ERRCLEANUP(err, 2);
  err = pthread_detach(t);
  ERRFATAL(err, "pthread_detach");

  // do not free spawn if call succeeds - will be freed in thunk
  goto error1;

error2:
  free(spawn);
  if(wg) {
    one_wait_group_done(wg); // will not be called by thunk
  }
error1:
  pthread_attr_destroy(&attr);
error0:
  return err;
}

