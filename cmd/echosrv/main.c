#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>

#include "oneone.h"
#include "errors.h"
#include "attributes.h"
#include "socket99/socket99.h"

void
serve_client(void * arg) {
  int * pfd = arg;
  char buf[100];
  int n = 1;

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

int
main(unused int argc, unused char ** argv) {
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

  while(true) {
    struct sockaddr addr;
    socklen_t addr_len;
    int client_fd = accept(res.fd, &addr, &addr_len);
    NEGFATAL(client_fd, "accept");

    int * pfd = malloc(sizeof(*pfd));
    *pfd = client_fd;
    int err = one_spawn_wg(wg, serve_client, pfd);
    ERRFATAL(err, "one_spawn_wg");
  }

  // TODO: add interrupt handler, graceful shutdown
  one_wait_group_free(wg);
}

