#include <pthread.h>

#include "oneone.h"
#include "errors.h"

typedef struct one_rwlocked_val_s {
  pthread_rwlock_t lock;
  void *val;
} one_rwlocked_val_s;

one_rwlocked_val_s *
one_rwlocked_val_new(void * const initial_val) {
  one_rwlocked_val_s * rwlv = malloc(sizeof(*rwlv));
  NULLFATAL(rwlv, "out of memory");

  int err = pthread_rwlock_init(&(rwlv->lock), NULL);
  ERRFATAL(err, "pthread_rwlock_init");
  rwlv->val = initial_val;

  return rwlv;
}

void *
one_rwlocked_val_free(one_rwlocked_val_s * const rwlv) {
  if(!rwlv) {
    return NULL;
  }

  void * val = rwlv->val;
  int err = pthread_rwlock_destroy(&(rwlv->lock));
  ERRFATAL(err, "pthread_rwlock_destroy");

  free(rwlv);
  return val;
}

void *
one_rwlocked_val_set(one_rwlocked_val_s * const rwlv, void * const new_val) {
  int err = 0;

  err = pthread_rwlock_wrlock(&(rwlv->lock));
  ERRFATAL(err, "pthread_rwlock_wrlock");

  void * old_val = rwlv->val;
  rwlv->val = new_val;

  err = pthread_rwlock_unlock(&(rwlv->lock));
  ERRFATAL(err, "pthread_rwlock_unlock");

  return old_val;
}

void *
one_rwlocked_val_get(one_rwlocked_val_s * const rwlv) {
  int err = 0;

  err = pthread_rwlock_rdlock(&(rwlv->lock));
  ERRFATAL(err, "pthread_rwlock_rdlock");

  void * val = rwlv->val;

  err = pthread_rwlock_unlock(&(rwlv->lock));
  ERRFATAL(err, "pthread_rwlock_unlock");

  return val;
}

void *
one_rwlocked_val_read_with(one_rwlocked_val_s * const rwlv, one_locked_val_read_fn fn, void * const arg) {
  int err = 0;

  err = pthread_rwlock_rdlock(&(rwlv->lock));
  ERRFATAL(err, "pthread_rwlock_rdlock");

  void * val = rwlv->val;
  fn(val, arg);

  err = pthread_rwlock_unlock(&(rwlv->lock));
  ERRFATAL(err, "pthread_rwlock_unlock");

  return val;
}

void *
one_rwlocked_val_with(one_rwlocked_val_s * const rwlv, one_locked_val_fn fn, void * const arg) {
  int err = 0;

  err = pthread_rwlock_wrlock(&(rwlv->lock));
  ERRFATAL(err, "pthread_rwlock_wrlock");

  void * val = rwlv->val;
  void * new_val = fn(val, arg);
  rwlv->val = new_val;

  err = pthread_rwlock_unlock(&(rwlv->lock));
  ERRFATAL(err, "pthread_rwlock_unlock");

  return new_val;
}

