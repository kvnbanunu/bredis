#include <arpa/inet.h>
#include <errno.h>
#include <netinet/ip.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Prints error message
static void msg(const char *msg) {
  fprintf(stderr, "%s\n", msg);
}

// Prints an error message and the last error code
// Intended to be used when the program should terminate due to an error
static void die(const char *msg) {
  int err = errno;
  fprintf(stderr, "[%d] %s\n", err, msg);
}

// Read & Write
static void do_something(int connfd) {
  char rbuf[64] = {};
  ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
  if (n < 0) {
    msg("read() error");
    return;
  }
  printf("client says: %s\n", rbuf);

  char wbuf[] = "world";
  write(connfd, wbuf, strlen(wbuf));
}

int main() {

  // Obtain a Socket Handle, IPv4, TCP
  int fd = socket(AF_INET, SOCK_STREAM, 0);

  // Configure socket options after the socket has been created
  int val = 1;
  setsockopt(fd,           // socket
             SOL_SOCKET,   // which option to set
             SO_REUSEADDR, // enable for every listening socket
             &val,         // option value
             sizeof(val)   // option value length
  );

  // Bind to an Address, IPv4
  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = ntohs(1234);
  addr.sin_addr.s_addr = ntohl(0); // wildcard address 0.0.0.0
  int rv = bind(fd, (const sockaddr *)&addr, sizeof(addr));
  if (rv) {
    die("bind()");
  }

  // Listen
  rv = listen(fd, SOMAXCONN); // SOMAXCONN is the size of the queue (128)
  if (rv) {
    die("listen()");
  }

  // Accept Connections
  while (true) {
    struct sockaddr_in client_addr = {};
    socklen_t addrlen = sizeof(client_addr); // input & output size
    int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen);
    if (connfd < 0) {
      continue; // error
    }

    do_something(connfd);
    close(connfd);
  }

  return 0;
}
