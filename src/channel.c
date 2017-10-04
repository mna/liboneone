#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>

#include "parallel.h"
#include "_errors.h"

typedef struct parallel_channel_s {
  int capacity;
  int count;

  sem_t sendsema;
  sem_t recvsema;

  pthread_mutex_t lock;
  void **values; // array of `capacity` void pointers
} parallel_channel_s;

parallel_channel_s * parallel_channel_new(int capacity) {
  parallel_channel_s *ch = malloc(sizeof(parallel_channel_s));
  // TODO: init other stuff...
  return ch;
}

void parallel_channel_free(parallel_channel_s *ch) {
  if(!ch) {
    return;
  }

  // TODO: release other stuff...
  free(ch);
}

void parallel_channel_send(parallel_channel_s *ch, void *value) {
  // acquire send sema (or block until), then store
  // similar to recv, except sendsema and count < capacity.
  // TODO: handle closed channel
}

void * parallel_channel_recv(parallel_channel_s *ch) {
  // TODO: handle closed channel... how?
  //
  // if something's available, return and incr sema
  // otherwise acquire (or block) recv sema
  pthread_mutex_lock(&(ch->lock));
  if(ch->count) {
    // return oldest item (FIFO)
  }
  pthread_mutex_unlock(&(ch->lock));

  // wait on ch->recvsema;
  // then repeat mutex/count
  return NULL;
}

void parallel_channel_close(parallel_channel_s *ch) {
  // how to signal all receivers?
}

