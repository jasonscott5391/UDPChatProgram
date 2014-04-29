
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFLEN 4096
#define IP "127.0.0.1"
#define PORT 1234
#define NAME "server"

int filegetline(char *, int, FILE *);
size_t getuserpass(char *, size_t, FILE *);

int main(int argc, char** argv) {

  FILE *fp = fopen("masterlist.txt", "a+");
  char* name = (char *)malloc(sizeof(char)* 16);
  char* password = (char *)malloc(sizeof(char) * 16);
  char* user = (char *)malloc(sizeof(char) * 16);
 
  if(argc > 2 ) {
    // Usage statement
  }
  
  if(argc == 2) {
    fprintf(stdout, "Enter new username: ");
    
    filegetline(name, 16, stdin);
    fprintf(stdout, "Enter password: ");
   
    getuserpass(password, 16, stdin);
    fprintf(stdout, "\n");

    strncpy(user, name, strlen(name)-1);
   
    fprintf(fp, "%s-%s", user, password);
    fclose(fp);
  }

  struct sockaddr_in my_addr, cli_addr;
  int sockfd;
  socklen_t slen = sizeof(cli_addr);
  char buf[BUFLEN];

  if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    fprintf(stderr, "Error: socket()\n");

  bzero(&my_addr, sizeof(my_addr));
  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(PORT);
  my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if(bind(sockfd, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1) {
    fprintf(stderr, "Error: bind()");
  }

  // Receive username and password for authentication
  int i = 1;
  while(i) {
    i=0;
    
  }
  

  // Messaging

  while(1) {

    // receive from client
    if(recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr*)&cli_addr, &slen) == -1) {
      fprintf(stderr, "Error: recvfrom()");
    }

    fprintf(stdout, "<%s>%s", user, buf);

    // send to client
    fprintf(stdout, "<%s>", NAME);
    filegetline(buf, BUFLEN, stdin);

    if(sendto(sockfd, buf, BUFLEN, 0, (struct sockaddr*)&cli_addr, slen) == -1) {
      fprintf(stderr, "Error: sendto()\n");
    }
  }
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

/* Reads a password by not displaying it on the screen */
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
