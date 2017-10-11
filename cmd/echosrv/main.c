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

// serves a single client in a thread.
void
serve_client(void * arg) {
  int * pfd = arg;
  char buf[100];
  int n = 1;

  printf("serving %d\n", *pfd);
  while(true) {
    n = read(*pfd, buf, sizeof(buf));
    NEGFATAL(n, "read");
    if(n == 0) {
      break;
    }

    n = write(*pfd, buf, n);
    NEGFATAL(n, "write");
  }

  printf("closing %d\n", *pfd);
  close(*pfd);
  free(pfd);
}

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

  one_wait_group_s * wg = one_wait_group_new(0);

  while(!stop_server) {
    struct sockaddr addr;
    socklen_t addr_len;
    int client_fd = accept(res.fd, &addr, &addr_len);
    if(client_fd == -1 && errno == EINTR) {
      continue;
    }
    NEGFATAL(client_fd, "accept");

    vec_push(&vec, client_fd);

    int * pfd = malloc(sizeof(*pfd));
    *pfd = client_fd;
    int err = one_spawn_wg(wg, serve_client, pfd);
    ERRFATAL(err, "one_spawn_wg");
  }
  close(res.fd);

  // TODO: threads must remove FD from vector when closed from client side
  int i; int val;
  vec_foreach(&vec, val, i) {
    int err = shutdown(val, SHUT_RD);
    NEGFATAL(err, "shutdown");
  }

  vec_deinit(&vec);
  one_wait_group_wait(wg);
  one_wait_group_free(wg);
}

