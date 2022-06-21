/*
 * Compilation:  gcc -Wall ./pcapfilter.c -o ./pcapfilter -lpcap
 * Usage:        ./pcapfilter INTERFACE
 * NOTE:         This program requires root privileges.
 */
#include <pcap.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <arpa/inet.h>

#define IP 6
#define UDP 17
#define	ETHTYPE_IP	0x0800		
#define ETHTYPE_ARP	0x0806		

char* errbuf;
pcap_t* handle;

struct counter{
  int arp;
  int ip;
  int tcp;
  int upd;
  int rest;
} counter={0,0,0,0,0};

void cleanup() {
  pcap_close(handle);
  free(errbuf);
}

void stop(int signo) {
  printf("\nARP:\t%d\nIP:\t%d\nIP/UDP:\t%d\nIP/TCP:\t%d\nRest:\t%d\n", 
  counter.arp, counter.ip, counter.upd, counter.tcp, counter.rest);

  exit(EXIT_SUCCESS);
}

void trap(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes) {
  struct ethhdr *eth;
  memcpy(&eth,&bytes,sizeof(bytes));
  struct iphdr *iph = (struct iphdr*)(bytes+  sizeof(struct ethhdr));
  struct iphdr *iph2 = (struct iphdr*)(bytes);

  char *bytearray = (char *)&iph;
  char *bytearray2 = (char *)&iph2;
    printf("\niph1\n");
  for (int i=0; i <sizeof(bytes);i++)
  {
    printf("%d",bytearray[i]);

  }
  printf("\niph2\n");
 for (int i=0; i <sizeof(bytes);i++)
  {
    printf("%d",bytearray2[i]);

  }

  if(ntohs(eth->h_proto) == ETHTYPE_ARP)
  {
    counter.arp++;
  }
  else if(ntohs(eth->h_proto) == ETHTYPE_IP)
  {
    counter.ip++;
    switch(iph->protocol){
      case IP:
        counter.tcp++;
        break;
      case UDP:
        counter.upd++;
        break;
      default: 
        counter.rest++;
        break;
    }
  }
  else
  {
    counter.rest++;
  }
}

int main(int argc, char** argv) {
  atexit(cleanup);
  signal(SIGINT, stop);
  errbuf = malloc(PCAP_ERRBUF_SIZE);
  handle = pcap_create(argv[1], errbuf);
  pcap_set_promisc(handle, 1);
  pcap_set_snaplen(handle, 65535);
  pcap_set_timeout(handle, 1000);
  pcap_activate(handle);
  pcap_loop(handle, -1, trap, NULL);
}
