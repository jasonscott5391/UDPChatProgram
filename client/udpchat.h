/* Copyright (C) 2014 Jason Scott */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BUFLEN 4096
#define IP "127.0.0.1"
#define PORT 1234

/* reads line from file */
size_t filegetline(char *line, size_t lim, FILE *fp)
{
  int c, i;
  for (i = 0; i < (lim-1) && (c = getc(fp)) != EOF && c != '\n'; ++i)
    line[i] = c;
  if (c == '\n')
    {
      line[i] = c;
      ++i;
    }
  line[i] = '\0';
  return i;
}

/* Reads a password by not displaying it on the screen */
size_t getuserpass(char *line, size_t lim, FILE *fp) {
  struct termios old, new;
  int nread;
  char *raw = (char *)malloc(sizeof(char) * 32);

  /* Turn echoing off and fail if we can't. */
  if (tcgetattr(fileno(fp), &old) != 0)
    return -1;
  new = old;
  new.c_lflag &= ~ECHO;
  if (tcsetattr(fileno(fp), TCSAFLUSH, &new) != 0)
    return -1;


  unsigned long seed[2];
  char salt[] = "$1$........";
  const char *const seedchars =
    "./0123456789ABCDEFGHIJKLMNOPQRST"
    "UVWXYZabcdefghijklmnopqrstuvwxyz";
  
  int i;
  
  seed[0] = time(NULL);
  seed[1] = getpid() ^ (seed[0] >> 14 & 0x30000);
  
  /* Turn it into printable characters from ‘seedchars’. */
  for (i = 0; i < 8; i++) {
    salt[3+i] = seedchars[(seed[i/5] >> (i%5)*6) & 0x3f];
  }

  /* Read the password. */
  nread = filegetline(raw, lim, fp);

  /* Encryption */
  strcpy(line, crypt(raw, salt));

  /* Restore terminal. */
  (void) tcsetattr (fileno(fp), TCSAFLUSH, &old);

  return nread;
}

