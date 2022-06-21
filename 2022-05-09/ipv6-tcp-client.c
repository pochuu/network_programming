/*
 * Compilation:  gcc -Wall ./ipv6-tcp-client.c -o ./ipv6-tcp-client
 * Usage:        ./ipv6-tcp-client SERVER PORT
 */

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int _connect(const char *host, const char *service) {
  int sfd;
  struct addrinfo hints, *res, *ressave;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  getaddrinfo(host, service, &hints, &res);
  ressave = res;
  do {
  sfd = socket(res->ai_family, res->ai_socktype,
  res->ai_protocol);
  if (connect(sfd, res->ai_addr, res->ai_addrlen) == 0)
  break;
  close(sfd);
  } while((res = res->ai_next) != NULL);
  freeaddrinfo(ressave);
return sfd;
}

int main(int argc, char** argv) {
  int sfd, rc;
  char buf[128];
  struct sockaddr_in6 saddr;

  sfd = socket(PF_INET6, SOCK_STREAM, 0);
  memset(&saddr, 0, sizeof(saddr));
  saddr.sin6_family = AF_INET6;
  saddr.sin6_port = htons(atoi(argv[2]));
  inet_pton(AF_INET6, argv[1], &saddr.sin6_addr);
  connect(sfd, (struct sockaddr*) &saddr, sizeof(saddr));
  rc = read(sfd, buf, 128);
  write(1, buf, rc);
  close(sfd);
  return EXIT_SUCCESS;
}


//2001:808:201:101:5ef9:ddff:fe78:5a6b Julian global
//fe80::5ef9:ddff:fe78:5a6b