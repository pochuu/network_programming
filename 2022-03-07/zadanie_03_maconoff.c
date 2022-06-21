#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
/*

 * Compilation:  gcc -Wall ./ifonoff.c -o ./ifonoff
 * Usage:        ./ifonoff INTERFACE SWITCH
 * SWITCH = 0 OR 1
 */

int main(int argc, char** argv) {
    int sfd,sw;
    struct ifreq ifr;

    sw = atoi(argv[2]);
    sfd = socket(PF_INET, SOCK_DGRAM, 0);
    strncpy(ifr.ifr_name, argv[1], IFNAMSIZ);

    if(sw == 1){
        ifr.ifr_flags |= IFF_UP;
    }
    else if(sw == 0){  
        ifr.ifr_flags &= ~IFF_UP;

    }
    else{
        printf("\n1 - interface on\n0 - interface off");
    }
    ioctl(sfd, SIOCSIFFLAGS, &ifr);
    close(sfd);
  return 0;
}