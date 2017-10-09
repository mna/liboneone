#include <pthread.h>

#include "oneone.h"
#include "errors.h"

typedef struct one_wait_group_s {
  pthread_cond_t cond;
  pthread_mutex_t lock;
  int count;
} one_wait_group_s;

one_wait_group_s *
one_wait_group_new(int initial_count) {
  one_wait_group_s * wg = malloc(sizeof(*wg));
  NULLFATAL(wg, "out of memory");

  int err = 0;

  err = pthread_cond_init(&(wg->cond), NULL);
  ERRFATAL(err, "pthread_cond_init");

  err = pthread_mutex_init(&(wg->lock), NULL);
  ERRFATAL(err, "pthread_mutex_init");

  wg->count = initial_count;
  NEGFATAL(wg->count, "negative wait group count");

  return wg;
}

void
one_wait_group_free(one_wait_group_s * const wg) {
  if(!wg) {
    return;
  }

  int err = 0;
  err = pthread_mutex_destroy(&(wg->lock));
  ERRFATAL(err, "pthread_mutex_destroy");
  err = pthread_cond_destroy(&(wg->cond));
  ERRFATAL(err, "pthread_cond_destroy");

  free(wg);
}

void
one_wait_group_add(one_wait_group_s * const wg, int delta) {
  if(delta == 0) {
    return;
  }

  int err = 0;

  err = pthread_mutex_lock(&(wg->lock));
  ERRFATAL(err, "pthread_mutex_lock");

  wg->count += delta;
  NEGFATAL(wg->count, "negative wait group count");

  if(wg->count == 0) {
    err = pthread_cond_broadcast(&(wg->cond));
    ERRFATAL(err, "pthread_cond_broadcast");
  }

  err = pthread_mutex_unlock(&(wg->lock));
  ERRFATAL(err, "pthread_mutex_unlock");
}

void
one_wait_group_done(one_wait_group_s * const wg) {
  one_wait_group_add(wg, -1);
}

void
one_wait_group_wait(one_wait_group_s * const wg) {
  int err = 0;

  err = pthread_mutex_lock(&(wg->lock));
  ERRFATAL(err, "pthread_mutex_lock");

  while(wg->count) {
    err = pthread_cond_wait(&(wg->cond), &(wg->lock));
    ERRFATAL(err, "pthread_cond_wait");
  }

  err = pthread_mutex_unlock(&(wg->lock));
  ERRFATAL(err, "pthread_mutex_unlock");
}

