/*
 * Compilation:  gcc -Wall ./ethrecv.c -o ./ethrecv
 * Usage:        ./ethrecv INTERFACE
 * NOTE:         This program requires root privileges.
 */

#include <arpa/inet.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define ETH_P_CUSTOM 0x8888

int main(int argc, char** argv) {
  int sfd, i;
  ssize_t len;
  char* frame;
  char* fdata;
  struct ethhdr* fhead;
  struct ifreq ifr;
  socklen_t sl;
  struct sockaddr_ll sall;
  struct sockaddr_ll sall2;

  sfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_CUSTOM));
  strncpy(ifr.ifr_name, argv[1], IFNAMSIZ);
  ioctl(sfd, SIOCGIFINDEX, &ifr);
  memset(&sall, 0, sizeof(struct sockaddr_ll));
  sall.sll_family = AF_PACKET;
  sall.sll_protocol = htons(ETH_P_CUSTOM);
  sall.sll_ifindex = ifr.ifr_ifindex;
  sall.sll_hatype = ARPHRD_ETHER;
  sall.sll_pkttype = PACKET_HOST;
  sall.sll_halen = ETH_ALEN;
  bind(sfd, (struct sockaddr*) &sall, sizeof(struct sockaddr_ll));
  while(1) {
    frame = malloc(ETH_FRAME_LEN);
    memset(frame, 0, ETH_FRAME_LEN);
    fhead = (struct ethhdr*) frame;
    fdata = frame + ETH_HLEN;
    len = recvfrom(sfd, frame, ETH_FRAME_LEN, 0, (struct sockaddr*) &sall2, &sl);
    printf("Frame length: %d\n",strlen(fdata));
    printf("[%dB] %02x:%02x:%02x:%02x:%02x:%02x -> ", (int)len,
           fhead->h_source[0], fhead->h_source[1], fhead->h_source[2],
           fhead->h_source[3], fhead->h_source[4], fhead->h_source[5]);
    printf("%02x:%02x:%02x:%02x:%02x:%02x | ",
           fhead->h_dest[0], fhead->h_dest[1], fhead->h_dest[2],
           fhead->h_dest[3], fhead->h_dest[4], fhead->h_dest[5]);
    printf("\nPKT TYPE: 0x%02x \n", sall2.sll_pkttype);
    printf("ETH TYPE: 0x%02x \n", fhead->h_proto);
    printf("MESSAGE: %s\n", fdata);
    printf("\n");
    for (i = 0; i < len ; i++) {
      printf("%02x ", (unsigned char) frame[i]);
      if ((i + 1) % 16 == 0)
        printf("\n");
    }
    free(frame);
  }
  close(sfd);
  return EXIT_SUCCESS;
}
