#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "parallel.h"
#include "_errors.h"

typedef struct parallel_wait_group_s {
  pthread_cond_t cond;
  pthread_mutex_t lock;
  int count;
} parallel_wait_group_s;

parallel_wait_group_s *
parallel_wait_group_new(int initial_count) {
  parallel_wait_group_s *wg = malloc(sizeof(parallel_wait_group_s));
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
parallel_wait_group_free(parallel_wait_group_s *wg) {
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
parallel_wait_group_add(parallel_wait_group_s *wg, int delta) {
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
parallel_wait_group_done(parallel_wait_group_s *wg) {
  parallel_wait_group_add(wg, -1);
}

void
parallel_wait_group_wait(parallel_wait_group_s *wg) {
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

