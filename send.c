// Wiktoria Kuna 316418
#include "traceroute.h"
int send_packet(int sockfd, char *ip_dest, u_int16_t seq, int ttl,
                struct icmp_response **responses) {

  pid_t pid = getpid();

  struct sockaddr_in recipient;
  bzero(&recipient, sizeof(recipient));
  recipient.sin_family = AF_INET;

  if (inet_pton(AF_INET, ip_dest, &recipient.sin_addr) < 0) {
    fprintf(stderr, "Invalid ip address\n");
    return EXIT_FAILURE;
  }

  struct icmp header;
  header.icmp_type = ICMP_ECHO;
  header.icmp_code = 0;
  header.icmp_hun.ih_idseq.icd_id = pid;
  header.icmp_hun.ih_idseq.icd_seq = seq;
  header.icmp_cksum = 0;
  header.icmp_cksum =
      compute_icmp_checksum((u_int16_t *)&header, sizeof(header));

  setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int));

  ssize_t bytes_sent = sendto(sockfd, &header, sizeof(header), 0,
                              (struct sockaddr *)&recipient, sizeof(recipient));

  gettimeofday(&responses[ttl][seq & 3].sent, NULL);
  
  if (bytes_sent == -1) {
    fprintf(stderr, "sendto: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}