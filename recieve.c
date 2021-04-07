// Wiktoria Kuna 316418
#include "traceroute.h"
int recieve_packet(int sockfd, struct icmp_response **responses,
                   int *responses_cnt, int ttl) {

  pid_t pid = getpid();

  fd_set descriptors;
  FD_ZERO(&descriptors);
  FD_SET(sockfd, &descriptors);
  struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec = 0;

  while (1) {
    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);
    u_int8_t buffer[IP_MAXPACKET];

    int ready = select(sockfd + 1, &descriptors, NULL, NULL, &tv);

    /* select error */
    if (ready < 0) {
      fprintf(stderr, "select error: %s\n", strerror(errno));
      return EXIT_FAILURE;
    }

    /* Exceeded time limit */
    if (ready == 0)
      return EXIT_SUCCESS;

    ssize_t packet_len = recvfrom(sockfd, buffer, IP_MAXPACKET, MSG_DONTWAIT,
                                  (struct sockaddr *)&sender, &sender_len);

    if (packet_len < 0) {
      fprintf(stderr, "recvfrom error: %s\n", strerror(errno));
      return EXIT_FAILURE;
    }

    char sender_ip[20];
    inet_ntop(AF_INET, &(sender.sin_addr), sender_ip, sizeof(sender_ip));

    struct ip *ip_header = (struct ip *)buffer;
    ssize_t ip_header_len = 4 * ip_header->ip_hl;

    struct icmp *icmp_header = (struct icmp *)(buffer + ip_header_len);
    
    /* ICMP specifics (documentation) */
    if (icmp_header->icmp_type == ICMP_TIME_EXCEEDED) {
      ip_header = (struct ip *)((void *)icmp_header + 8);
      ip_header_len = 4 * ip_header->ip_hl;
      icmp_header = (struct icmp *)((void *)ip_header + ip_header_len);
    }

   /* Filtering out packets from outside this particular program */
    if (icmp_header->icmp_hun.ih_idseq.icd_id == pid) {
      int packet_ttl = icmp_header->icmp_hun.ih_idseq.icd_seq >> 2,
          packet_no = icmp_header->icmp_hun.ih_idseq.icd_seq & 3;

      /* Filtering out packets that are not from current iteration */
      if (packet_ttl == ttl) { 
        gettimeofday(&responses[ttl][packet_no].recieved, NULL);
        strncpy(responses[ttl][packet_no].src_addr, sender_ip,
                sizeof(responses[ttl][packet_no].src_addr));
        responses_cnt[ttl]++;
      }
    }
  }

  return EXIT_SUCCESS;
}
