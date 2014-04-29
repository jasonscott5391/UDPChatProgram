#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFLEN 4096
#define IP "127.0.0.1"
#define PORT 1234

int filegetline(char *, int, FILE *);
size_t getuserpass(char *, size_t, FILE *);

int main(int argc, char** argv) {
    
  char *password = (char *)malloc(sizeof(char) * 16);
  
  if(argc != 2) {
    fprintf(stderr, "Usage: [username]\n");
    exit(0);
  }
  fprintf(stdout, "Password: ");
  // filegetline(password, 16, stdin);
  getuserpass(password, 16, stdin);

  struct sockaddr_in serv_addr;
  int sockfd;
  socklen_t slen = sizeof(serv_addr);
  char buf[BUFLEN];

  if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    fprintf(stderr, "Error: socket()\n");
  bzero(&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);
  
  if(inet_aton(IP, &serv_addr.sin_addr) == 0) {
    fprintf(stderr, "Error: inet_aton()\n");
    exit(1);
  }

  

  // Send username and password for authentication
  int i = 1;
  while(i) {
    i = 0;
  }

  // Messaging
  while(1) {
    fprintf(stdout, "<%s>", argv[1]);

    // get message from standard in
    filegetline(buf, BUFLEN, stdin);

    // send message to server
    if(sendto(sockfd, buf, BUFLEN, 0, (struct sockaddr*)&serv_addr, slen) == -1) {
      fprintf(stderr, "Error: sendto()\n");
    }

    // receive message from server
    if(recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr*)&serv_addr, &slen) == -1) {
     fprintf(stderr, "Error: recvfrom()");
     }

    fprintf(stdout, "<server>%s", buf);
  } 
   close(sockfd);
   return 0;

}

/* reads line from file */
int filegetline(char s[],int lim, FILE *fp)
{
  int c, i;
  for (i = 0; i < (lim-1) && (c = getc(fp)) != EOF && c != '\n'; ++i)
    s[i] = c;
  if (c == '\n')
    {
      s[i] = c;
      ++i;
    }
  s[i] = '\0';
  return i;
}

size_t getuserpass(char *line, size_t lim, FILE *fp) {
  struct termios old, new;
  int nread;
     
  /* Turn echoing off and fail if we can't. */
  if (tcgetattr(fileno(fp), &old) != 0)
    return -1;
  new = old;
  new.c_lflag &= ~ECHO;
  if (tcsetattr(fileno(fp), TCSAFLUSH, &new) != 0)
    return -1;
     
  /* Read the password. */
  nread = filegetline(line, lim, fp);
     
  /* Restore terminal. */
  (void) tcsetattr (fileno(fp), TCSAFLUSH, &old);
     
  return nread;
}
