/* Copyright (C) 2014 Jason Scott */
#include "udpchat.h"

#define NAME "server"

int main(int argc, char** argv) {

  FILE *fp;
  char *name = (char *)malloc(sizeof(char)* 32);
  char *password = (char *)malloc(sizeof(char) * 32);
  char *confirm = (char *)malloc(sizeof(char) * 32);
  char *user = (char *)malloc(sizeof(char) * 32);
  char *auth = (char *)malloc(sizeof(char));
  
  
  if(argc > 2 ) {
    fprintf(stderr, "Usage: %s [-n]\n n: Create new user.\n", argv[0]);
    exit(0);
  }

  // Add new user
  if(argc == 2) {
    fprintf(stdout, "Enter new username: ");
    
    filegetline(name, 32, stdin);
    fprintf(stdout, "Enter password: ");
   
    getuserpass(password, 32, stdin);
    fprintf(stdout, "\nPlease confirm your password.\nEnter password: ");

    getuserpass(confirm, 32, stdin);
    fprintf(stdout, "\n");

    if((strncmp(password, confirm, strlen(password))) != 0) {
      fprintf(stderr, "Passwords do not match!\n");
      exit(0);
    }
    
    strncpy(user, name, strlen(name)-1);
    
    if((fp = fopen("masterlist.txt", "a+")) == NULL) {
      fprintf(stderr, "Error opening master file!\n");
      exit(0);
    }

    fprintf(fp, "%s-%s\n", user, password);
    fclose(fp);
    
    fprintf(stdout, "Success! %s has been added.\n", user);
  }
  
  fprintf(stdout, "Awaiting connection request...\n");

  struct sockaddr_in my_addr, cli_addr;
  int sockfd;
  socklen_t slen = sizeof(cli_addr);
  char buf[BUFLEN];

  if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    fprintf(stderr, "\nError: socket()\n");

  bzero(&my_addr, sizeof(my_addr));
  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(PORT);
  my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if(bind(sockfd, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1) {
    fprintf(stderr, "\nError: bind()\n");
  }

  // Receive username and password for authentication
  int i = 1;
  user = (char *)realloc(user, 16);
  password = (char *)realloc(password, 16);
  int len;
  char *line = (char *)malloc(sizeof(char) * 16);
  
  // Authentication loop
  while(i) {
    
    // Receive username & password
    if(recvfrom(sockfd, user, 32, 0, (struct sockaddr*)&cli_addr, &slen) == -1) {
      fprintf(stderr, "\nError: username recvfrom()\n");
    }

    // Receive password
    if(recvfrom(sockfd, password, 16, 0, (struct sockaddr*)&cli_addr, &slen) == -1) {
      fprintf(stderr, "\nError: password recvfrom()\n");
    }
    
    // Authenticate login credentials
    if((fp = fopen("masterlist.txt", "r")) == NULL) {
      fprintf(stderr, "\nError opening master file.\n");
      exit(0);
    }

    // Combine user and password
    int keylen = strlen(user) + strlen(password);
    char *key = (char *)malloc(sizeof(char) * keylen);
    strncat(key, user, strlen(user));
    strncat(key, "-", 1);
    strncat(key, password, strlen(password));
      
    // Read master text file searching for correct username and password 
    while((len = filegetline(line, 32, fp)) > 0) {
      if(strncmp(key, line, strlen(key)) == 0) {
	// Send a 1 for success
	auth = "1";
	fprintf(stdout, "\n%s has connected...\n", user);
	i = 0;
      }	
    }
    if(sendto(sockfd, auth, 16, 0, (struct sockaddr*)&cli_addr, slen) == -1) {
      fprintf(stderr, "Error: sendto()\n");
    }
    
  }
  // Messaging loop
  while(1) {
    
    fprintf(stdout, "Awaiting message from %s\n", user);
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

