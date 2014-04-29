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

  FILE *fp;
  char* name = (char *)malloc(sizeof(char)* 16);
  char* password = (char *)malloc(sizeof(char) * 16);
  char* user = (char *)malloc(sizeof(char) * 16);
 
  if(argc > 2 ) {
    fprintf(stderr, "Usage: %s [-n]\n n: Create new user.\n", argv[0]);
    exit(0);
  }

  // Add new user
  if(argc == 2) {
    fprintf(stdout, "Enter new username: ");
    
    filegetline(name, 16, stdin);
    fprintf(stdout, "Enter password: ");
   
    getuserpass(password, 16, stdin);
    fprintf(stdout, "\n");

    strncpy(user, name, strlen(name)-1);
    
    if((fp = fopen("masterlist.txt", "a+")) == NULL) {
      fprintf(stderr, "Error opening master file!\n");
      exit(0);
    }

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
  user = (char *)realloc(user, 16);
  password = (char *)realloc(password, 16);
  int len;
  char *line = (char *)malloc(sizeof(char) * 16);

  // Authentication loop
  while(i) {
    
    // Receive username
    if(recvfrom(sockfd, user, 16, 0, (struct sockaddr*)&cli_addr, &slen) == -1) {
      fprintf(stderr, "Error: username recvfrom()");
    }

    // Receive password
    if(recvfrom(sockfd, password, 16, 0, (struct sockaddr*)&cli_addr, &slen) == -1) {
      fprintf(stderr, "Error: password recvfrom()");
    }

    // Authenticate login credentials
    if((fp = fopen("masterlist.txt", "r")) == NULL) {
      fprintf(stderr, "Error opening master file.\n");
      exit(0);
    }
    
    // Read master text file searching for correct username and password 
    while((len = filegetline(line, 32, fp)) > 0) {
                 
       if(strncmp(user, line, strlen(user)) == 0) {
	
	 // Send a 1 for success
      	if(sendto(sockfd, "1", 16, 0, (struct sockaddr*)&cli_addr, slen) == -1) {
      	  fprintf(stderr, "Error: sendto()\n");
      	}
      	fprintf(stdout, "%s has connected...\n", user);
      	i = 0;
       }
    }
  }
  
  // Messaging loop
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
