#include <unistd.h>

#include "oneone.h"
#include "errors.h"
#include "greatest/greatest.h"

static void
spawn_chan(void *arg) {
  one_chan_s * ch = arg;

  int * value;
  int err = one_chan_recv(ch, (void **)&value);
  ERRFATAL(err, "one_chan_recv");
  int recvd = *value;
  free(value);

  int * ret = malloc(sizeof(*ret));
  *ret = recvd + 1;

  err = one_chan_send(ch, ret);
  ERRFATAL(err, "one_chan_send");
}

TEST
test_send_recv() {
  one_wait_group_s * wg = one_wait_group_new(0);
  one_chan_s * ch = one_chan_new();

  one_spawn_wg(wg, spawn_chan, ch);

  int * psend = malloc(sizeof(*psend));
  *psend = 42;
  int err = one_chan_send(ch, psend);
  ERRFATAL(err, "one_chan_send");

  int * recv = NULL;
  err = one_chan_recv(ch, (void **)&recv);
  ERRFATAL(err, "one_chan_recv");
  int got = *recv;
  free(recv);

  ASSERT_EQ_FMT(43, got, "%d");

  one_wait_group_wait(wg);

  one_chan_free(ch);
  one_wait_group_free(wg);

  PASS();
}

static void
spawn_senda(void *arg) {
  one_chan_s * ch = arg;
  one_chan_send(ch, "a");
}

static void
spawn_sendb(void *arg) {
  one_chan_s *ch = arg;
  one_chan_send(ch, "b");
}

TEST
test_block_recv_multi_send() {
  one_wait_group_s * wg = one_wait_group_new(0);
  one_chan_s * ch = one_chan_new();

  one_spawn_wg(wg, spawn_senda, ch);
  one_spawn_wg(wg, spawn_sendb, ch);
  usleep(10000); // let spawned threads block on send

  char * recvd;
  int err = one_chan_recv(ch, (void **)&recvd);
  ERRFATAL(err, "one_chan_recv");
  ASSERT(strcmp(recvd, "a") == 0 || strcmp(recvd, "b") == 0);

  char * next = strcmp(recvd, "a") == 0 ? "b" : "a";
  err = one_chan_recv(ch, (void **)&recvd);
  ERRFATAL(err, "one_chan_recv");
  ASSERT_STR_EQ(next, recvd);

  one_wait_group_wait(wg);
  one_chan_free(ch);
  one_wait_group_free(wg);

  PASS();
}

static void
spawn_blocked_send(void * arg) {
  one_chan_s * ch = arg;
  one_chan_send(ch, "a");
}

TEST
test_close_with_blocked_sender() {
  one_wait_group_s * wg = one_wait_group_new(0);
  one_chan_s * ch = one_chan_new();

  one_spawn_wg(wg, spawn_blocked_send, ch);
  usleep(10000); // let spawned thread block on send

  int err = one_chan_close(ch);
  ERRFATAL(err, "one_chan_close");

  one_wait_group_wait(wg);
  one_chan_free(ch);
  one_wait_group_free(wg);

  PASS();
}

static void
spawn_blocked_recv(void *arg) {
  one_chan_s * ch = arg;
  char * recvd;
  one_chan_recv(ch, (void **)&recvd);
}

TEST
test_close_with_blocked_receiver() {
  one_wait_group_s * wg = one_wait_group_new(0);
  one_chan_s * ch = one_chan_new();

  one_spawn_wg(wg, spawn_blocked_recv, ch);
  usleep(10000); // let spawned thread block on send

  int err = one_chan_close(ch);
  ERRFATAL(err, "one_chan_close");

  one_wait_group_wait(wg);
  one_chan_free(ch);
  one_wait_group_free(wg);

  PASS();
}

TEST
test_chan_new() {
  one_chan_s * ch = one_chan_new();
  ASSERT(ch);
  one_chan_free(ch);

  PASS();
}

TEST
test_chan_close() {
  one_chan_s * ch = one_chan_new();
  int err = one_chan_close(ch);
  ASSERT_EQ(ESUCCESS, err);

  one_chan_free(ch);

  PASS();
}

SUITE(chan) {
  RUN_TEST(test_chan_new);
  RUN_TEST(test_chan_close);
  RUN_TEST(test_send_recv);
  RUN_TEST(test_block_recv_multi_send);
  RUN_TEST(test_close_with_blocked_sender);
  RUN_TEST(test_close_with_blocked_receiver);
}

