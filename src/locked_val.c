#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "parallel.h"
#include "_errors.h"

typedef struct parallel_locked_val_s {
  pthread_mutex_t lock;
  void *val;
} parallel_locked_val_s;

parallel_locked_val_s * parallel_locked_val_new(void *initial_val) {
  parallel_locked_val_s *lv = malloc(sizeof(parallel_locked_val_s));
  NULLFATAL(lv, "out of memory");

  int err = pthread_mutex_init(&(lv->lock), NULL);
  ERRFATAL(err, "pthread_mutex_init");
  lv->val = initial_val;

  return lv;
}

void * parallel_locked_val_free(parallel_locked_val_s *lv) {
  if(!lv) {
    return NULL;
  }

  void *val = lv->val;
  int err = pthread_mutex_destroy(&(lv->lock));
  ERRFATAL(err, "pthread_mutex_destroy");

  free(lv);
  return val;
}

void * parallel_locked_val_set(parallel_locked_val_s *lv, void *new_val) {
  int err = 0;

  err = pthread_mutex_lock(&(lv->lock));
  ERRFATAL(err, "pthread_mutex_lock");

  void *old_val = lv->val;
  lv->val = new_val;

  err = pthread_mutex_unlock(&(lv->lock));
  ERRFATAL(err, "pthread_mutex_unlock");

  return old_val;
}

void * parallel_locked_val_get(parallel_locked_val_s *lv) {
  int err = 0;

  err = pthread_mutex_lock(&(lv->lock));
  ERRFATAL(err, "pthread_mutex_lock");

  void *val = lv->val;

  err = pthread_mutex_unlock(&(lv->lock));
  ERRFATAL(err, "pthread_mutex_unlock");

  return val;
}

void * parallel_locked_val_with(parallel_locked_val_s *lv, void *(*fn) (void *)) {
  int err = 0;

  err = pthread_mutex_lock(&(lv->lock));
  ERRFATAL(err, "pthread_mutex_lock");

  void *val = lv->val;
  void *new_val = fn(val);
  lv->val = new_val;

  err = pthread_mutex_unlock(&(lv->lock));
  ERRFATAL(err, "pthread_mutex_unlock");

  return new_val;
}

