/*
 * Compilation:  gcc -Wall ./tcp-server-thread.c -o ./tcp-server-thread
 * Usage:        ./tcp-server-thread
 */

#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#define ERROR(e) { perror(e); exit(EXIT_FAILURE); }
#define SERVER_PORT 1234
#define QUEUE_SIZE 5

struct cln {
  int cfd;
  struct sockaddr_in caddr;
};

void* cthread(void* arg) {
  struct cln* c = (struct cln*)arg;
  printf("new connection: %s\n", inet_ntoa((struct in_addr)c->caddr.sin_addr));
  if (write(c->cfd, "Hello World!\n", 13) < 0)
    ERROR("write()")
  sleep(10);
  close(c->cfd);
  free(c);
  return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
  pthread_t tid;
  socklen_t slt;
  int sfd, on = 1;
  struct sockaddr_in saddr;

  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = INADDR_ANY;
  saddr.sin_port = htons(SERVER_PORT);

  if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    ERROR("socket()")
  if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) < 0)
    ERROR("setsockopt()")
  if (bind(sfd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0)
    ERROR("bind()")
  if (listen(sfd, QUEUE_SIZE) < 0)
    ERROR("listen()")

  while(1) {
    struct cln* c = malloc(sizeof(struct cln));
    slt = sizeof(c->caddr);
    if ((c->cfd = accept(sfd, (struct sockaddr*)&c->caddr, &slt)) < 0)
      ERROR("accept()")

    if (pthread_create(&tid, NULL, cthread, c) < 0)
      ERROR("pthread_create()")
    pthread_detach(tid);
  }

  close(sfd);
  return EXIT_SUCCESS;
}
