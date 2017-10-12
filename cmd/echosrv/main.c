#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <signal.h>

#include "oneone.h"
#include "errors.h"
#include "attributes.h"
#include "socket99/socket99.h"
#include "vec/src/vec.h"

// set in signal handlers to signal graceful termination of server.
static bool stop_server = false;

void
signal_handler(unused int signal) {
  stop_server = true;
}

void
register_signal_handlers() {
  struct sigaction sa;
  sa.sa_handler = signal_handler;
  sa.sa_flags = 0; // do not automatically retry, we want EINTR
  sigfillset(&sa.sa_mask);

  int err = sigaction(SIGINT, &sa, NULL);
  NEGFATAL(err, "sigaction");
}

void *
push_client_fd(void * val, void * const arg) {
  vec_int_t * vec = val;
  int * pfd = arg;

  vec_push(vec, *pfd);
  return vec;
}

void *
pop_client_fd(void * val, void * const arg) {
  vec_int_t * vec = val;
  int * pfd = arg;

  vec_remove(vec, *pfd);
  return vec;
}

typedef struct client_arg_s {
  int fd;
  one_locked_val_s * locked_vec;
} client_arg_s;

// serves a single client in a thread.
void
serve_client(void * arg) {
  client_arg_s * ca = arg;

  char buf[100];
  int n = 1;

  printf("serving %d\n", ca->fd);
  one_locked_val_with(ca->locked_vec, push_client_fd, &(ca->fd));

  while(true) {
    n = read(ca->fd, buf, sizeof(buf));
    NEGFATAL(n, "read");
    if(n == 0) {
      break;
    }

    n = write(ca->fd, buf, n);
    NEGFATAL(n, "write");
  }

  printf("closing %d\n", ca->fd);
  close(ca->fd);
  one_locked_val_with(ca->locked_vec, pop_client_fd, &(ca->fd));
  free(ca);
}

void *
shutdown_client_fds(void * val, unused void * const arg) {
  vec_int_t * vec = val;

  int i; int v;
  vec_foreach(vec, v, i) {
    int err = shutdown(v, SHUT_RD);
    NEGFATAL(err, "shutdown");
  }

  return vec;
}

int
main(unused int argc, unused char ** argv) {
  register_signal_handlers();

  int reuse_addr_val = 1;
  socket99_sockopt opt = {
    .option_id = SO_REUSEADDR,
    .value = &reuse_addr_val,
    .value_len = sizeof(reuse_addr_val),
  };

  socket99_config cfg = {
    .host = "127.0.0.1",
    .port = 8080,
    .server = true,
    .sockopts[0] = opt,
  };

  socket99_result res;
  bool ok = socket99_open(&cfg, &res);
  if(!ok) {
    char buf[100];
    socket99_snprintf(buf, sizeof(buf), &res);
    FATAL(buf);
  }

  vec_int_t vec;
  vec_init(&vec);

  one_locked_val_s * locked_vec = one_locked_val_new(&vec);
  one_wait_group_s * wg = one_wait_group_new(0);

  while(!stop_server) {
    struct sockaddr addr;
    socklen_t addr_len;
    int client_fd = accept(res.fd, &addr, &addr_len);
    if(client_fd == -1 && errno == EINTR) {
      continue;
    }
    NEGFATAL(client_fd, "accept");

    client_arg_s * arg = malloc(sizeof(*arg));
    arg->fd = client_fd;
    arg->locked_vec = locked_vec;

    int err = one_spawn_wg(wg, serve_client, arg);
    ERRFATAL(err, "one_spawn_wg");
  }
  close(res.fd);

  one_locked_val_with(locked_vec, shutdown_client_fds, NULL);

  one_wait_group_wait(wg);
  one_wait_group_free(wg);
  one_locked_val_free(locked_vec);
  vec_deinit(&vec);
}

