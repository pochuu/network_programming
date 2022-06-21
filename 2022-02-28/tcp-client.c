/*
 * Compilation:  gcc -Wall ./tcp-client.c -o ./tcp-client
 * Usage:        ./tcp-client SERVER PORT
 */

#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char** argv) {
  int sfd;
  char text[]="Test\n";
  struct sockaddr_in saddr; 
  struct hostent* addrent;
  addrent = gethostbyname(argv[1]);
  sfd = socket(PF_INET, SOCK_STREAM, 0); 
  memset(&saddr, 0, sizeof(saddr)); 
  saddr.sin_family = AF_INET; 
  saddr.sin_port = htons(atoi(argv[2])); 
  memcpy(&saddr.sin_addr.s_addr, addrent->h_addr, addrent->h_length); 

  connect(sfd, (struct sockaddr*) &saddr, sizeof(saddr)); 
  write(sfd, text, strlen(text));

  
  close(sfd);
  return EXIT_SUCCESS;
}
