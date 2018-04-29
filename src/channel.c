#include <pthread.h>
#include <stdbool.h>

#include "errors.h"
#include "oneone.h"

typedef enum waiter_signaled_e {
  ws_false = 0,
  ws_true = 1,
  ws_closed = 99,
} waiter_signaled_e;

typedef struct chan_waiter_s {
  pthread_mutex_t lock;
  pthread_cond_t cond;
  waiter_signaled_e signaled;
  void* val;
  struct chan_waiter_s* next;
} chan_waiter_s;

typedef struct one_chan_s {
  pthread_mutex_t lock;
  bool closed;
  chan_waiter_s* qsend;
  chan_waiter_s* qrecv;
} one_chan_s;

static chan_waiter_s* chan_waiter_new(void* value) {
  chan_waiter_s* const w = malloc(sizeof(*w));
  NULLFATAL(w, "out of memory");

  w->signaled = ws_false;
  w->val = value;
  w->next = NULL;

  int err = 0;
  err = pthread_mutex_init(&(w->lock), NULL);
  ERRFATAL(err, "pthread_mutex_init");

  err = pthread_cond_init(&(w->cond), NULL);
  ERRFATAL(err, "pthread_cond_init");

  return w;
}

static void chan_waiter_free(chan_waiter_s* const w) {
  if (!w) {
    return;
  }

  int err = 0;
  err = pthread_mutex_destroy(&(w->lock));
  ERRFATAL(err, "pthread_mutex_destroy");
  err = pthread_cond_destroy(&(w->cond));
  ERRFATAL(err, "pthread_cond_destroy");
  free(w);
}

static void chan_waiter_append(chan_waiter_s** list, chan_waiter_s* const w) {
  if (!(*list)) {
    *list = w;
    return;
  }

  chan_waiter_s* p;
  for (p = *list; p->next; p = p->next)
    ;
  p->next = w;
}

static void chan_waiter_signal(chan_waiter_s* const w, bool chan_closed) {
  int err = 0;

  err = pthread_mutex_lock(&(w->lock));
  ERRFATAL(err, "pthread_mutex_lock");

  w->signaled = chan_closed ? ws_closed : ws_true;

  err = pthread_cond_signal(&(w->cond));
  ERRFATAL(err, "pthread_cond_signal");

  err = pthread_mutex_unlock(&(w->lock));
  ERRFATAL(err, "pthread_mutex_unlock");
}

static void* chan_waiter_block(chan_waiter_s* const w) {
  int err = 0;

  err = pthread_mutex_lock(&(w->lock));
  ERRFATAL(err, "pthread_mutex_lock");

  while (!w->signaled) {
    err = pthread_cond_wait(&(w->cond), &(w->lock));
    ERRFATAL(err, "pthread_cond_wait");
  }

  void* value = w->val;
  err = pthread_mutex_unlock(&(w->lock));
  ERRFATAL(err, "pthread_mutex_unlock");

  // waiter is removed from the channel's queue already, only
  // needs to be freed.
  chan_waiter_free(w);
  return value;
}

one_chan_s* one_chan_new() {
  one_chan_s* ch = malloc(sizeof(*ch));

  ch->closed = false;
  ch->qsend = NULL;
  ch->qrecv = NULL;

  int merr = pthread_mutex_init(&(ch->lock), NULL);
  ERRFATAL(merr, "pthread_mutex_init");

  return ch;
}

void one_chan_free(one_chan_s* const ch) {
  if (!ch) {
    return;
  }

  int merr = pthread_mutex_lock(&(ch->lock));
  ERRFATAL(merr, "pthread_mutex_lock");

  // should not be called with pending waiters
  if (ch->qsend || ch->qrecv) {
    FATAL("parallel_channel_free called with pending waiters");
  }

  merr = pthread_mutex_unlock(&(ch->lock));
  ERRFATAL(merr, "pthread_mutex_unlock");

  merr = pthread_mutex_destroy(&(ch->lock));
  ERRFATAL(merr, "pthread_mutex_destroy");

  free(ch);
}

int one_chan_send(one_chan_s* const ch, void* value) {
  int err = ESUCCESS;

  int merr = pthread_mutex_lock(&(ch->lock));
  ERRFATAL(merr, "pthread_mutex_lock");

  if (ch->closed) {
    err = ECLOSEDCHAN;
    goto error1;
  }

  // if there is a receiver waiting, send the value immediately
  if (ch->qrecv) {
    chan_waiter_s* r = ch->qrecv;
    r->val = value;
    ch->qrecv = r->next;
    chan_waiter_signal(r, false);
    goto error1;
  }

  // otherwise add the sender to the waiting list
  chan_waiter_s* s = chan_waiter_new(value);
  chan_waiter_append(&(ch->qsend), s);

  // unlock the channel
  merr = pthread_mutex_unlock(&(ch->lock));
  ERRFATAL(merr, "pthread_mutex_unlock");

  // block the send waiter until a receiver is ready
  chan_waiter_block(s);
  if (s->signaled == ws_closed) {
    err = ECLOSEDCHAN;
  }
  goto error0;

error1:
  merr = pthread_mutex_unlock(&(ch->lock));
  ERRFATAL(merr, "pthread_mutex_unlock");
error0:
  return err;
}

int one_chan_recv(one_chan_s* const ch, void** value) {
  int err = ESUCCESS;

  int merr = pthread_mutex_lock(&(ch->lock));
  ERRFATAL(merr, "pthread_mutex_lock");

  if (ch->closed) {
    err = ECLOSEDCHAN;
    goto error1;
  }

  // if there is a sender waiting, receive the value immediately
  if (ch->qsend) {
    chan_waiter_s* s = ch->qsend;
    *value = s->val;
    ch->qsend = s->next;
    chan_waiter_signal(s, false);
    goto error1;
  }

  // otherwise add the receiver to the waiting list
  chan_waiter_s* r = chan_waiter_new(NULL);
  chan_waiter_append(&(ch->qrecv), r);

  // unlock the channel
  merr = pthread_mutex_unlock(&(ch->lock));
  ERRFATAL(merr, "pthread_mutex_unlock");

  // block the receive waiter until a sender is ready
  void* recvd = chan_waiter_block(r);
  *value = recvd;
  if (r->signaled == ws_closed) {
    err = ECLOSEDCHAN;
    *value = NULL;
  }

  goto error0;

error1:
  merr = pthread_mutex_unlock(&(ch->lock));
  ERRFATAL(merr, "pthread_mutex_unlock");
error0:
  return err;
}

int one_chan_close(one_chan_s* const ch) {
  int err = ESUCCESS;

  // lock the channel
  int merr = pthread_mutex_lock(&(ch->lock));
  ERRFATAL(merr, "pthread_mutex_lock");

  if (ch->closed) {
    err = ECLOSEDCHAN;
    goto error1;
  }

  ch->closed = true;
  chan_waiter_s* s = ch->qsend;
  chan_waiter_s* r = ch->qrecv;
  ch->qsend = NULL;
  ch->qrecv = NULL;

  // can unlock now, impossible to add new waiters with channel closed
  merr = pthread_mutex_unlock(&(ch->lock));
  ERRFATAL(merr, "pthread_mutex_unlock");

  // signal all waiters
  for (chan_waiter_s* w = s; w; w = w->next) {
    chan_waiter_signal(w, true);
  }
  for (chan_waiter_s* w = r; w; w = w->next) {
    chan_waiter_signal(w, true);
  }

  goto error0;

error1:
  merr = pthread_mutex_unlock(&(ch->lock));
  ERRFATAL(merr, "pthread_mutex_unlock");
error0:
  return err;
}
