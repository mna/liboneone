#include <pthread.h>

#include "oneone.h"
#include "errors.h"

typedef struct one_locked_val_s {
  pthread_mutex_t lock;
  void *val;
} one_locked_val_s;

one_locked_val_s *
one_locked_val_new(void * const initial_val) {
  one_locked_val_s * lv = malloc(sizeof(*lv));
  NULLFATAL(lv, "out of memory");

  int err = pthread_mutex_init(&(lv->lock), NULL);
  ERRFATAL(err, "pthread_mutex_init");
  lv->val = initial_val;

  return lv;
}

void *
one_locked_val_free(one_locked_val_s * const lv) {
  if(!lv) {
    return NULL;
  }

  void * val = lv->val;
  int err = pthread_mutex_destroy(&(lv->lock));
  ERRFATAL(err, "pthread_mutex_destroy");

  free(lv);
  return val;
}

void *
one_locked_val_set(one_locked_val_s * const lv, void * const new_val) {
  int err = 0;

  err = pthread_mutex_lock(&(lv->lock));
  ERRFATAL(err, "pthread_mutex_lock");

  void * old_val = lv->val;
  lv->val = new_val;

  err = pthread_mutex_unlock(&(lv->lock));
  ERRFATAL(err, "pthread_mutex_unlock");

  return old_val;
}

void *
one_locked_val_get(one_locked_val_s * const lv) {
  int err = 0;

  err = pthread_mutex_lock(&(lv->lock));
  ERRFATAL(err, "pthread_mutex_lock");

  void * val = lv->val;

  err = pthread_mutex_unlock(&(lv->lock));
  ERRFATAL(err, "pthread_mutex_unlock");

  return val;
}

void *
one_locked_val_with(one_locked_val_s * const lv, one_locked_val_fn fn) {
  int err = 0;

  err = pthread_mutex_lock(&(lv->lock));
  ERRFATAL(err, "pthread_mutex_lock");

  void * val = lv->val;
  void * new_val = fn(val);
  lv->val = new_val;

  err = pthread_mutex_unlock(&(lv->lock));
  ERRFATAL(err, "pthread_mutex_unlock");

  return new_val;
}

