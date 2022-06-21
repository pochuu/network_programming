/*
 * Compilation:  gcc -Wall ./pcapsniff.c -o ./pcapsniff -lpcap
 * Usage:        ./pcapsniff INTERFACE
 * NOTE:         This program requires root privileges.
 */

#include <pcap.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

char* errbuf;
pcap_t* handle;

void cleanup() {
  pcap_close(handle);
  free(errbuf);
}

void stop(int signo) {
  exit(EXIT_SUCCESS);
}

void trap(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes) {
  printf("[%dB of %dB]\n", h->caplen, h->len);
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
