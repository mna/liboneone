#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <signal.h>

#include "oneone.h"
#include "errors.h"
#include "attributes.h"
#include "socket99/socket99.h"

#define VECTOR_TYPE int
#define VECTOR_NAME int
#include "vector.h/vector.h"

static bool stop_server = false;

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
  sa.sa_flags = SA_RESTART;
  sigfillset(&sa.sa_mask);

  int err = sigaction(SIGINT, &sa, NULL);
  NEGFATAL(err, "sigaction");
}

int
main(unused int argc, unused char ** argv) {
  register_signal_handlers();

  socket99_config cfg = {
    .host = "127.0.0.1",
    .port = 8080,
    .server = true,
  };

  socket99_result res;
  bool ok = socket99_open(&cfg, &res);
  if(!ok) {
    char buf[100];
    socket99_snprintf(buf, sizeof(buf), &res);
    FATAL(buf);
  }

  one_wait_group_s * wg = one_wait_group_new(0);

  vec_int_t client_fds;
  vec_int_init(client_fds);

  while(!stop_server) {
    struct sockaddr addr;
    socklen_t addr_len;
    int client_fd = accept(res.fd, &addr, &addr_len);
    NEGFATAL(client_fd, "accept");
    vec_int_append(client_fds, client_fd);

    int * pfd = malloc(sizeof(*pfd));
    *pfd = client_fd;
    int err = one_spawn_wg(wg, serve_client, pfd);
    ERRFATAL(err, "one_spawn_wg");
  }

  while(vec_int_size(client_fds)) {
    int fd = vec_int_pop(client_fds);
    printf("shutting down %d\n", fd);
    int err = shutdown(fd, SHUT_RDWR);
    NEGFATAL(err, "shutdown");
  }

  one_wait_group_wait(wg);
  one_wait_group_free(wg);
}

