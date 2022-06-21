#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#define ERROR(e) { perror(e); exit(EXIT_FAILURE); }
#define SERVER_PORT 1234
#define QUEUE_SIZE 5

int main(int argc, char** argv) {
  socklen_t slt;
  int sfd, cfd, on = 1;
  int epfd, nrinq;
  struct sockaddr_in saddr, caddr;
  struct epoll_event epool, events[QUEUE_SIZE];

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
  if ((epfd= epoll_create1(0))==-1)
    ERROR("epoll_createl()")

  epool.events = EPOLLIN;
  epool.data.fd = sfd;

  if(epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, &epool) == -1)
    ERROR("epoll_ctrl()")

  while(1) {
    if ((nrinq=epoll_wait(epfd, events, QUEUE_SIZE, -1)) < 0)
      ERROR("epoll_wait()")

    for(int i = 0; i<nrinq;i++){
       slt = sizeof(caddr);
       if ((cfd = accept(sfd, (struct sockaddr *)&caddr, &slt)) < 0)
          ERROR("accept()")
       printf("client ip: %s \n",inet_ntoa(caddr.sin_addr));;
       write(cfd, "Hello World!123\n", 16);
    }
  }

  close(sfd);
  return EXIT_SUCCESS;
}
