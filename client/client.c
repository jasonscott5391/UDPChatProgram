#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFLEN 4096
#define IP "127.0.0.1"
#define PORT 1234

int main(int argc, char** argv) {
  
  struct sockaddr_in serv_addr;
  int sockfd;
  socklen_t slen = sizeof(serv_addr);
  char buf[BUFLEN];
  char *line = (char *)malloc(sizeof(char) * 16);

  if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    fprintf(stderr, "Error: socket()\n");
  bzero(&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);
  
  if(inet_aton(IP, &serv_addr.sin_addr) == 0) {
    fprintf(stderr, "Error: inet_aton()\n");
    exit(1);
  }

  if(argc != 2) {
    fprintf(stderr, "Usage: [username]\n");
    exit(0);
  }
  fprintf(stdout, "Password: ");
  fgets(line, sizeof(line), stdin);

  // Send username and password for authentication

  // Messaging
  while(1) {
    // get message from standard in
    fgets(buf, BUFLEN, stdin);

    // send message to server
    if(sendto(sockfd, buf, BUFLEN, 0, (struct sockaddr*)&serv_addr, slen) == -1) {
      fprintf(stderr, "Error: sendto()\n");
    }

    // receive message from server
    if(recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr*)&serv_addr, &slen) == -1) {
     fprintf(stderr, "Error: recvfrom()");
     }

    fprintf(stdout, "%s", buf);
  } 
   close(sockfd);
   return 0;

}
