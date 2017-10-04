#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include "parallel.h"
#include "_errors.h"

static size_t _get_default_stack_size() {
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

typedef struct _fnf_s {
  void (*fn) (void *);
  void *arg;
  parallel_wait_group_s *wg;
} _fnf_s;

static void *_fnf_thunk(void *arg) {
  _fnf_s *fnf = arg;
  if(fnf) {
    parallel_wait_group_s *wg = fnf->wg;
    fnf->fn(fnf->arg);
    free(fnf);

    if(wg) {
      parallel_wait_group_done(wg);
    }
  }
  return NULL;
}

int parallel_fnf(void (*fn) (void *), void *arg) {
  return parallel_fnf_wg_ssz(NULL, fn, arg, _get_default_stack_size());
}

int parallel_fnf_wg(parallel_wait_group_s *wg, void (*fn) (void *), void *arg) {
  return parallel_fnf_wg_ssz(wg, fn, arg, _get_default_stack_size());
}

int parallel_fnf_ssz(void (*fn) (void *), void *arg, size_t stack_sz) {
  return parallel_fnf_wg_ssz(NULL, fn, arg, stack_sz);
}

int parallel_fnf_wg_ssz(parallel_wait_group_s *wg, void (*fn) (void *), void *arg, size_t stack_sz) {
  pthread_attr_t attr;
  pthread_t t;
  int err = 0;

  // configure the thread's stack size
  err = pthread_attr_init(&attr);
  ERRCLEANUP(err, 0);
  err = pthread_attr_setstacksize(&attr, stack_sz);
  ERRCLEANUP(err, 1);

  // create the thunk to call fn with the signature expected by pthread
  _fnf_s *fnf = malloc(sizeof(_fnf_s));
  NULLFATAL(fnf, "out of memory");
  fnf->fn = fn;
  fnf->arg = arg;
  fnf->wg = wg;

  // at this point the wait group must be incremented
  if(wg) {
    parallel_wait_group_add(wg, 1);
  }

  // create the detached thread
  err = pthread_create(&t, &attr, _fnf_thunk, fnf);
  ERRCLEANUP(err, 2);
  err = pthread_detach(t);
  ERRFATAL(err, "pthread_detach");

  // do not free fnf if call succeeds - will be freed in thunk
  goto error1;

error2:
  free(fnf);
  parallel_wait_group_done(wg); // will not be called by thunk
error1:
  pthread_attr_destroy(&attr);
error0:
  return err;
}

