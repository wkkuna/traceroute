// Wiktoria Kuna 316418
#ifndef TRACEROUTE
#define TRACEROUTE
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

struct icmp_response {
  char src_addr[20];
  struct timeval sent;
  struct timeval recieved;
};

u_int16_t compute_icmp_checksum(const void *buff, int length);
void print_route(struct icmp_response **responses, int ttl, int *responses_cnt);
int recieve_packet(int sockfd, struct icmp_response **responses,
                   int *responses_cnt, int ttl);
int send_packet(int sockfd, char *ip_dest, u_int16_t seq, int ttl,
                struct icmp_response **responses);

#endif