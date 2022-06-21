/*
 * Compilation:  gcc -Wall ./tcp-server.c -o ./tcp-server
 * Usage:        ./tcp-server SERVER PORT
 */

#include <netinet/in.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char** argv) {
  socklen_t sl; 
  int s2fd,sfd, portno, rc, cfd, on = 1;
  char buf[128];
  struct hostent* addrent;
  struct sockaddr_in s2addr, saddr, caddr; 
  
  addrent = gethostbyname(argv[1]);
  portno = atoi(argv[2]);
  memset(&s2addr, 0, sizeof(s2addr));
  s2addr.sin_family = AF_INET; 
  memcpy(&s2addr.sin_addr.s_addr, addrent->h_addr, addrent->h_length);  
  s2addr.sin_port = htons(portno);
  s2fd = socket(PF_INET, SOCK_STREAM, 0);

  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET; 
  saddr.sin_addr.s_addr = INADDR_ANY; 
  saddr.sin_port = htons(1234); 
  sfd = socket(PF_INET, SOCK_STREAM, 0); 
  setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char*) &on, sizeof(on));
      
  bind(sfd, (struct sockaddr*) &saddr, sizeof(saddr)); 
  listen(sfd, 5); 
  
  while(1){
    //odbieranie
    memset(&caddr, 0, sizeof(caddr));
    sl = sizeof(caddr);
    cfd = accept(sfd, (struct sockaddr*) &caddr, &sl); 
    rc = read(cfd, buf, 128);
    write(1, buf, rc);
    close(cfd);

    //wysylanie
    connect(s2fd, (struct sockaddr*) &s2addr, sizeof(s2addr)); 
    write(s2fd, buf, rc);
    close(s2fd);
  }

  close(sfd);
  return EXIT_SUCCESS;
}
