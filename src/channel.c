#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

#include "parallel.h"
#include "_errors.h"

typedef struct _channel_waiter_s {
  pthread_mutex_t lock;
  pthread_cond_t cond;
  bool signaled;
  void *val;
  struct _channel_waiter_s *next;
} _channel_waiter_s;

typedef struct parallel_channel_s {
  pthread_mutex_t lock;
  bool closed;
  _channel_waiter_s *qsend;
  _channel_waiter_s *qrecv;
} parallel_channel_s;

static _channel_waiter_s *
_new_waiter(void *value) {
  _channel_waiter_s *w = malloc(sizeof(_channel_waiter_s));
  NULLFATAL(w, "out of memory");
  w->signaled = false;
  w->val = value;
  w->next = NULL;

  int err = 0;
  err = pthread_mutex_init(&(w->lock), NULL);
  ERRFATAL(err, "pthread_mutex_init");

  err = pthread_cond_init(&(w->cond), NULL);
  ERRFATAL(err, "pthread_cond_init");

  return w;
}

static void
_free_waiter(_channel_waiter_s *w) {
  if(!w) {
    return;
  }

  int err = 0;
  err = pthread_mutex_destroy(&(w->lock));
  ERRFATAL(err, "pthread_mutex_destroy");
  err = pthread_cond_destroy(&(w->cond));
  ERRFATAL(err, "pthread_cond_destroy");
  free(w);
}

static void
_append_waiter(_channel_waiter_s **list, _channel_waiter_s *w) {
  if(!*list) {
    *list = w;
    return;
  }

  _channel_waiter_s *p;
  for (p = *list; p->next; p = p->next)
    ;
  p->next = w;
}

static void
_signal_waiter(_channel_waiter_s *w) {
  int err = 0;

  err = pthread_mutex_lock(&(w->lock));
  ERRFATAL(err, "pthread_mutex_lock");

  w->signaled = true;

  err = pthread_cond_signal(&(w->cond));
  ERRFATAL(err, "pthread_cond_signal");

  err = pthread_mutex_unlock(&(w->lock));
  ERRFATAL(err, "pthread_mutex_unlock");
}

static void *
_block_waiter(_channel_waiter_s *w) {
  int err = 0;

  err = pthread_mutex_lock(&(w->lock));
  ERRFATAL(err, "pthread_mutex_lock");

  while(!w->signaled) {
    err = pthread_cond_wait(&(w->cond), &(w->lock));
    ERRFATAL(err, "pthread_cond_wait");
  }

  void *value = w->val;
  err = pthread_mutex_unlock(&(w->lock));
  ERRFATAL(err, "pthread_mutex_unlock");

  // waiter is removed from the channel's queue already, only
  // needs to be freed.
  _free_waiter(w);
  return value;
}

parallel_channel_s *
parallel_channel_new() {
  parallel_channel_s *ch = malloc(sizeof(parallel_channel_s));
  ch->closed = false;
  ch->qsend = NULL;
  ch->qrecv = NULL;

  int merr = pthread_mutex_init(&(ch->lock), NULL);
  ERRFATAL(merr, "pthread_mutex_init");

  return ch;
}

void
parallel_channel_free(parallel_channel_s *ch) {
  if(!ch) {
    return;
  }

  int merr = pthread_mutex_lock(&(ch->lock));
  ERRFATAL(merr, "pthread_mutex_lock");

  // should not be called with pending waiters
  if(ch->qsend || ch->qrecv) {
    FATAL("parallel_channel_free called with pending waiters");
  }

  merr = pthread_mutex_unlock(&(ch->lock));
  ERRFATAL(merr, "pthread_mutex_unlock");

  merr = pthread_mutex_destroy(&(ch->lock));
  ERRFATAL(merr, "pthread_mutex_destroy");

  free(ch);
}

int
parallel_channel_send(parallel_channel_s *ch, void *value) {
  int err = ESUCCESS;

  int merr = pthread_mutex_lock(&(ch->lock));
  ERRFATAL(merr, "pthread_mutex_lock");

  if(ch->closed) {
    err = ECLOSEDCHAN;
    goto error1;
  }

  // if there is a receiver waiting, send the value immediately
  if(ch->qrecv) {
    _channel_waiter_s *r = ch->qrecv;
    r->val = value;
    ch->qrecv = r->next;
    _signal_waiter(r);
    goto error1;
  }

  // otherwise add the sender to the waiting list
  _channel_waiter_s *s = _new_waiter(value);
  _append_waiter(&(ch->qsend), s);

  // unlock the channel
  merr = pthread_mutex_unlock(&(ch->lock));
  ERRFATAL(merr, "pthread_mutex_unlock");

  // block the send waiter until a receiver is ready
  // TODO: should return an int err and set value on an arg.
  // or (because _block_waiter cannot check closed on channel),
  // return a sentinel value (pointer to a static var) representing
  // closed channel. Can check here if it is that value and
  // return ECLOSEDCHAN.
  _block_waiter(s);
  goto error0;

error1:
  merr = pthread_mutex_unlock(&(ch->lock));
  ERRFATAL(merr, "pthread_mutex_unlock");
error0:
  return err;
}

int
parallel_channel_recv(parallel_channel_s *ch, void **value) {
  int err = ESUCCESS;

  int merr = pthread_mutex_lock(&(ch->lock));
  ERRFATAL(merr, "pthread_mutex_lock");

  if(ch->closed) {
    err = ECLOSEDCHAN;
    goto error1;
  }

  // if there is a sender waiting, receive the value immediately
  if(ch->qsend) {
    _channel_waiter_s *s = ch->qsend;
    *value = s->val;
    ch->qsend = s->next;
    _signal_waiter(s);
    goto error1;
  }

  // otherwise add the receiver to the waiting list
  _channel_waiter_s *r = _new_waiter(NULL);
  _append_waiter(&(ch->qrecv), r);

  // unlock the channel
  merr = pthread_mutex_unlock(&(ch->lock));
  ERRFATAL(merr, "pthread_mutex_unlock");

  // block the receive waiter until a sender is ready
  // TODO: should return an int err and set value on an arg.
  // or (because _block_waiter cannot check closed on channel),
  // return a sentinel value (pointer to a static var) representing
  // closed channel. Can check here if it is that value and
  // return ECLOSEDCHAN.
  void *recvd = _block_waiter(r);
  *value = recvd;

  goto error0;

error1:
  merr = pthread_mutex_unlock(&(ch->lock));
  ERRFATAL(merr, "pthread_mutex_unlock");
error0:
  return err;
}

int
parallel_channel_close(parallel_channel_s *ch) {
  int err = ESUCCESS;

  // lock the channel
  int merr = pthread_mutex_lock(&(ch->lock));
  ERRFATAL(merr, "pthread_mutex_lock");

  if(ch->closed) {
    err = ECLOSEDCHAN;
    goto error1;
  }

  ch->closed = true;
  _channel_waiter_s *s = ch->qsend;
  _channel_waiter_s *r = ch->qrecv;
  ch->qsend = NULL;
  ch->qrecv = NULL;

  // can unlock now, impossible to add new waiters with channel closed
  merr = pthread_mutex_unlock(&(ch->lock));
  ERRFATAL(merr, "pthread_mutex_unlock");

  // signal all waiters
  for(_channel_waiter_s *w = s; w; w = w->next) {
    _signal_waiter(w);
  }
  for(_channel_waiter_s *w = r; w; w = w->next) {
    _signal_waiter(w);
  }

  goto error0;

error1:
  merr = pthread_mutex_unlock(&(ch->lock));
  ERRFATAL(merr, "pthread_mutex_unlock");
error0:
  return err;
}

