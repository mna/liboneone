#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "parallel.h"
#include "_errors.h"

typedef struct parallel_rwlocked_val_s {
  pthread_rwlock_t lock;
  void *val;
} parallel_rwlocked_val_s;

parallel_rwlocked_val_s *
parallel_rwlocked_val_new(void *initial_val) {
  parallel_rwlocked_val_s *rwlv = malloc(sizeof(parallel_rwlocked_val_s));
  NULLFATAL(rwlv, "out of memory");

  int err = pthread_rwlock_init(&(rwlv->lock), NULL);
  ERRFATAL(err, "pthread_rwlock_init");
  rwlv->val = initial_val;

  return rwlv;
}

void *
parallel_rwlocked_val_free(parallel_rwlocked_val_s *rwlv) {
  if(!rwlv) {
    return NULL;
  }

  void *val = rwlv->val;
  int err = pthread_rwlock_destroy(&(rwlv->lock));
  ERRFATAL(err, "pthread_rwlock_destroy");

  free(rwlv);
  return val;
}

void *
parallel_rwlocked_val_set(parallel_rwlocked_val_s *rwlv, void *new_val) {
  int err = 0;

  err = pthread_rwlock_wrlock(&(rwlv->lock));
  ERRFATAL(err, "pthread_rwlock_wrlock");

  void *old_val = rwlv->val;
  rwlv->val = new_val;

  err = pthread_rwlock_unlock(&(rwlv->lock));
  ERRFATAL(err, "pthread_rwlock_unlock");

  return old_val;
}

void *
parallel_rwlocked_val_get(parallel_rwlocked_val_s *rwlv) {
  int err = 0;

  err = pthread_rwlock_rdlock(&(rwlv->lock));
  ERRFATAL(err, "pthread_rwlock_rdlock");

  void *val = rwlv->val;

  err = pthread_rwlock_unlock(&(rwlv->lock));
  ERRFATAL(err, "pthread_rwlock_unlock");

  return val;
}

void *
parallel_rwlocked_val_rdwith(parallel_rwlocked_val_s *rwlv, parallel_locked_rdval_func fn) {
  int err = 0;

  err = pthread_rwlock_rdlock(&(rwlv->lock));
  ERRFATAL(err, "pthread_rwlock_rdlock");

  void *val = rwlv->val;
  fn(val);

  err = pthread_rwlock_unlock(&(rwlv->lock));
  ERRFATAL(err, "pthread_rwlock_unlock");

  return val;
}

void *
parallel_rwlocked_val_wrwith(parallel_rwlocked_val_s *rwlv, parallel_locked_val_func fn) {
  int err = 0;

  err = pthread_rwlock_wrlock(&(rwlv->lock));
  ERRFATAL(err, "pthread_rwlock_wrlock");

  void *val = rwlv->val;
  void *new_val = fn(val);
  rwlv->val = new_val;

  err = pthread_rwlock_unlock(&(rwlv->lock));
  ERRFATAL(err, "pthread_rwlock_unlock");

  return new_val;
}

