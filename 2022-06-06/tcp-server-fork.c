/*
 * Compilation:  gcc -Wall ./tcp-server-fork.c -o ./tcp-server-fork
 * Usage:        ./tcp-server-fork
 */

#include <netdb.h>
#include <signal.h>
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

void childend(int signo) {
  printf("Childend \n");
  wait(NULL);

}

int main(int argc, char** argv) {
  socklen_t slt;
  int sfd, cfd, on = 1;
  struct sockaddr_in saddr, caddr;

  //signal(SIGCHLD, childend);

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
    slt = sizeof(caddr);
    if ((cfd = accept(sfd, (struct sockaddr*)&caddr, &slt)) < 0)
      ERROR("accept()")

    if (!fork()) {
      close(sfd);
      printf("new connection: %s\n",
             inet_ntoa((struct in_addr)caddr.sin_addr));
      if (write(cfd, "Hello World!\n", 13) < 0)
        ERROR("write()")
      close(cfd);
      exit(EXIT_SUCCESS);
    }
    close(cfd);
  }

  close(sfd);
  return EXIT_SUCCESS;
}
