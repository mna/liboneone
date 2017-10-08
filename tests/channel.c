#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../src/parallel.h"
#include "../src/_errors.h"
#include "../deps/greatest/greatest.h"

static void
_spawn_chan(void *arg) {
  parallel_channel_s *ch = arg;

  int *value;
  int err = parallel_channel_recv(ch, (void **)&value);
  ERRFATAL(err, "parallel_channel_recv");
  int recvd = *value;
  free(value);

  int *ret = malloc(sizeof(int));
  *ret = recvd + 1;

  err = parallel_channel_send(ch, ret);
  ERRFATAL(err, "parallel_channel_send");
}

TEST
test_send_recv() {
  parallel_wait_group_s *wg = parallel_wait_group_new(0);
  parallel_channel_s *ch = parallel_channel_new();

  parallel_spawn_wg(wg, _spawn_chan, ch);

  int *psend = malloc(sizeof(int));
  *psend = 42;
  int err = parallel_channel_send(ch, psend);
  ERRFATAL(err, "parallel_channel_send");

  int *recv = NULL;
  err = parallel_channel_recv(ch, (void **)&recv);
  ERRFATAL(err, "parallel_channel_recv");
  int got = *recv;
  free(recv);

  ASSERT_EQ_FMT(43, got, "%d");

  parallel_wait_group_wait(wg);

  parallel_channel_free(ch);
  parallel_wait_group_free(wg);

  PASS();
}

static void
_spawn_senda(void *arg) {
  parallel_channel_s *ch = arg;
  parallel_channel_send(ch, "a");
}

static void
_spawn_sendb(void *arg) {
  parallel_channel_s *ch = arg;
  parallel_channel_send(ch, "b");
}

TEST
test_block_recv_multi_send() {
  parallel_wait_group_s *wg = parallel_wait_group_new(0);
  parallel_channel_s *ch = parallel_channel_new();

  parallel_spawn_wg(wg, _spawn_senda, ch);
  parallel_spawn_wg(wg, _spawn_sendb, ch);
  usleep(100000); // let spawned threads block on send

  char *recvd;
  int err = parallel_channel_recv(ch, (void **)&recvd);
  ERRFATAL(err, "parallel_channel_recv");
  ASSERT(strcmp(recvd, "a") == 0 || strcmp(recvd, "b") == 0);

  char *next = strcmp(recvd, "a") == 0 ? "b" : "a";
  err = parallel_channel_recv(ch, (void **)&recvd);
  ERRFATAL(err, "parallel_channel_recv");
  ASSERT_STR_EQ(next, recvd);

  parallel_wait_group_wait(wg);
  parallel_channel_free(ch);
  parallel_wait_group_free(wg);

  PASS();
}

static void
_spawn_blocked_send(void *arg) {
  parallel_channel_s *ch = arg;
  parallel_channel_send(ch, "a");
}

TEST
test_close_with_blocked_sender() {
  parallel_wait_group_s *wg = parallel_wait_group_new(0);
  parallel_channel_s *ch = parallel_channel_new();

  parallel_spawn_wg(wg, _spawn_blocked_send, ch);
  usleep(100000); // let spawned thread block on send

  int err = parallel_channel_close(ch);
  ERRFATAL(err, "parallel_channel_close");

  parallel_wait_group_wait(wg);
  parallel_channel_free(ch);
  parallel_wait_group_free(wg);

  PASS();
}

static void
_spawn_blocked_recv(void *arg) {
  parallel_channel_s *ch = arg;
  char *recvd;
  parallel_channel_recv(ch, (void **)&recvd);
}

TEST
test_close_with_blocked_receiver() {
  parallel_wait_group_s *wg = parallel_wait_group_new(0);
  parallel_channel_s *ch = parallel_channel_new();

  parallel_spawn_wg(wg, _spawn_blocked_recv, ch);
  usleep(100000); // let spawned thread block on send

  int err = parallel_channel_close(ch);
  ERRFATAL(err, "parallel_channel_close");

  parallel_wait_group_wait(wg);
  parallel_channel_free(ch);
  parallel_wait_group_free(wg);

  PASS();
}

TEST
test_channel_new() {
  parallel_channel_s *ch = parallel_channel_new();
  ASSERT(ch);
  parallel_channel_free(ch);

  PASS();
}

TEST
test_channel_close() {
  parallel_channel_s *ch = parallel_channel_new();
  int err = parallel_channel_close(ch);
  ASSERT_EQ(ESUCCESS, err);

  parallel_channel_free(ch);

  PASS();
}

SUITE(channel) {
  RUN_TEST(test_channel_new);
  RUN_TEST(test_channel_close);
  RUN_TEST(test_send_recv);
  RUN_TEST(test_block_recv_multi_send);
  RUN_TEST(test_close_with_blocked_sender);
  RUN_TEST(test_close_with_blocked_receiver);
}

