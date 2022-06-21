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
#include <linux/route.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <linux/if.h>

#define ETH_P_CUSTOM 0x8888

struct ifrtinfo {
  int iri_type;
  char iri_iname[16];
  struct sockaddr_in iri_iaddr; /* IP address */
  struct sockaddr_in iri_rtdst; /* dst. IP address */
  struct sockaddr_in iri_rtmsk; /* dst. netmask */
  struct sockaddr_in iri_rtgip; /* gateway IP */
};

int main(int argc, char** argv) {
  int sfd, i; //file descriptor dla socketu
  ssize_t len;
  int sfd2;
  struct ifreq ifr5; //dzieki temu mozemy uzywać wykonywać ioctl dla interfejsu
  struct sockaddr_in *sin2,*addr;

  struct rtentry route;
  struct ifrtinfo* info;
  char* frame;
  char* fdata;
  struct ethhdr* fhead;
  struct ifreq ifr,ifr2;
  struct sockaddr_ll sall;
   printf("cos \n");
  sfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_CUSTOM)); //network byte order
  strncpy(ifr.ifr_name, argv[1], IFNAMSIZ); //kopiuje nazwe interfejsu
  ioctl(sfd, SIOCGIFINDEX, &ifr); //wstawia index interfejsu do ifr_ifindex
  memset(&sall, 0, sizeof(struct sockaddr_ll)); //zeruje strukture
  sall.sll_family = AF_PACKET; //gniazdo typu pf_packet
  sall.sll_protocol = htons(ETH_P_CUSTOM); //jakiego protokolu mamy odbierac wiadomosci
  sall.sll_ifindex = ifr.ifr_ifindex; //z linijki 53 odbieramy indeks interfejsu
     printf("cos2 \n");
  sall.sll_hatype = ARPHRD_ETHER; //10mbps tehrente
  sall.sll_pkttype = PACKET_HOST; //pakiety są tylko dla nas
  sall.sll_halen = ETH_ALEN; //ilosc oktaw w 1 eth addr
     printf("cos3 \n");
  bind(sfd, (struct sockaddr*) &sall, sizeof(struct sockaddr_ll)); //przypisujemy adres socketowi
       printf("cos4 \n");
  while(1) {
    frame = malloc(ETH_FRAME_LEN); //zaalokowanie 1514 bajtow
    memset(frame, 0, ETH_FRAME_LEN); //wypelnienei 0 
    fhead = (struct ethhdr*) frame;
    fdata = frame + ETH_HLEN;
    len = recvfrom(sfd, frame, ETH_FRAME_LEN, 0, NULL, NULL);

    info = (struct ethhdr*) (frame+14);
    printf("iri_type=%d\n",info->iri_type);
    
    switch(info->iri_type)
    {
      case 0:
          sfd2 = socket(PF_INET, SOCK_DGRAM, 0);
          strncpy(ifr5.ifr_name, info->iri_iname, strlen(info->iri_iname) + 1);
          sin2 = (struct sockaddr_in*) &ifr5.ifr_addr;
          memset(sin2, 0, sizeof(struct sockaddr_in));
          sin2->sin_family = info->iri_iaddr.sin_family;
          sin2->sin_port = info->iri_iaddr.sin_port;
          sin2->sin_addr.s_addr = info->iri_iaddr.sin_addr.s_addr;
          ioctl(sfd2, SIOCSIFADDR, &ifr5);
          ioctl(sfd2, SIOCGIFFLAGS, &ifr5);
          ifr5.ifr_flags |= IFF_UP | IFF_RUNNING;
          ioctl(sfd2, SIOCSIFFLAGS, &ifr5);
          close(sfd2);

      case 1:
            sfd2 = socket(PF_INET, SOCK_DGRAM, 0);
            memset(&route, 0, sizeof(route));

            addr = (struct sockaddr_in*) &route.rt_gateway;
            addr->sin_family = AF_INET;
            addr->sin_addr.s_addr = info->iri_rtgip.sin_addr.s_addr;

            addr = (struct sockaddr_in*) &route.rt_dst;
            addr->sin_family = AF_INET;
            addr->sin_addr.s_addr = INADDR_ANY;

            addr = (struct sockaddr_in*) &route.rt_genmask;
            addr->sin_family = AF_INET;
            addr->sin_addr.s_addr = INADDR_ANY;
            route.rt_flags = RTF_UP | RTF_GATEWAY;
            route.rt_metric = 0;
            ioctl(sfd2, SIOCADDRT, &route);
            
            close(sfd2);

      default:
      printf("error\n");
    }
    /*
    printf("[%dB] %02x:%02x:%02x:%02x:%02x:%02x -> ", (int)len,
           fhead->h_source[0], fhead->h_source[1], fhead->h_source[2],
           fhead->h_source[3], fhead->h_source[4], fhead->h_source[5]);
    printf("%02x:%02x:%02x:%02x:%02x:%02x | ",
           fhead->h_dest[0], fhead->h_dest[1], fhead->h_dest[2],
           fhead->h_dest[3], fhead->h_dest[4], fhead->h_dest[5]);
    printf("%s\n", fdata);
    for (i = 0; i < len ; i++) {
      printf("%02x ", (unsigned char) frame[i]);
      if ((i + 1) % 16 == 0)
        printf("\n");
    }
    printf("\n\n");
    free(frame);*/
  }
  close(sfd);
  return EXIT_SUCCESS;
}
