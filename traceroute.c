// Wiktoria Kuna 316418
#include "traceroute.h"

u_int16_t compute_icmp_checksum(const void *buff, int length) {
  u_int32_t sum;
  const u_int16_t *ptr = buff;
  assert(length % 2 == 0);
  for (sum = 0; length > 0; length -= 2)
    sum += *ptr++;
  sum = (sum >> 16) + (sum & 0xffff);
  return (u_int16_t)(~(sum + (sum >> 16)));
}

void print_route(struct icmp_response **responses, int ttl,
                 int *responses_cnt) {

  printf("%d. ", ttl);

  char *a1 = responses[ttl][0].src_addr, *a2 = responses[ttl][1].src_addr,
       *a3 = responses[ttl][2].src_addr;

  /* If packets came from different addrs*/
  if (a1[0])
    printf("%s ", a1);

  if (a2[0] && strcmp(a1, a2))
    printf("%s ", a2);

  if (a3[0] && strcmp(a3, a1) && strcmp(a3, a2))
    printf("%s ", a3);

  if (responses_cnt[ttl] == 0) { /* No response recieved */
    printf("*\n");
  } else if (responses_cnt[ttl] != 3) { /* Not all responses came on time */
    printf("???\n");
  } else {
    struct timeval t, avg; /* Average rtt */
    avg.tv_sec = avg.tv_usec = 0;
    for (int i = 0; i < 3; i++) {
      timersub(&responses[ttl][0].recieved, &responses[ttl][0].sent, &t);
      timeradd(&t, &avg, &avg);
    }

    size_t avg_rtt = avg.tv_sec * 1000 + avg.tv_usec / 1000;
    avg_rtt /= 3;

    printf("%ldms\n", avg_rtt);
  }
}

int traceroute(int sockfd, char *ip_dest, struct icmp_response **responses,
               int *responses_cnt) {
  

  for (int ttl = 1; ttl <= 30; ttl++) {
    for (int i = 0; i < 3; i++) {
      if (send_packet(sockfd, ip_dest, ttl << 2 | i, ttl, responses) ==
          EXIT_FAILURE)
        return EXIT_FAILURE;
    }

    if (recieve_packet(sockfd, responses, responses_cnt, ttl) == EXIT_FAILURE)
      return EXIT_FAILURE;

    print_route(responses, ttl, responses_cnt);

    /* Target reached*/
    if (!strcmp(ip_dest, responses[ttl][0].src_addr) ||
        !strcmp(ip_dest, responses[ttl][1].src_addr) ||
        !strcmp(ip_dest, responses[ttl][2].src_addr))
      return EXIT_SUCCESS;
  }

  return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
  if (argc != 2)
    return -1;

  int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

  if (sockfd < 0) {
    fprintf(stderr, "socket error: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }

  /* Initialization */
  struct icmp_response **responses = malloc(31 * sizeof(struct icmp_response *));

  for (int i = 1; i <= 31; i++) {
    responses[i] = malloc(3 * sizeof(struct icmp_response));
    for (int j = 0; j < 3; j++) {
      responses[i][j].recieved.tv_sec = 0;
      responses[i][j].recieved.tv_usec = 0;
      responses[i][j].src_addr[0] = '\0';
    }
  }

  int *responses_cnt = calloc(sizeof(int), 30);

  return traceroute(sockfd, argv[1], responses, responses_cnt);
}