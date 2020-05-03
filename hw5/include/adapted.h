/**
 * This file is basically a subset of CSAPP.H. I used it to adapt
 * from functions that were efficient in facilitating client to server connection.
 *
 * NOTE: some functions may be slightly modified from the original files for my own
 * convienence and to fit precisely what I am trying to accomplish.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*****************************************
 * Parts extracted from CSAPP.H & My Own
 *****************************************/

typedef struct sockaddr SA;
int open_clientfd(char *hostname, int port);
int open_listenfd(int port);
int convertStr2Int(char *message);

/* External variables */
extern int h_errno;    /* Defined by BIND for DNS errors */
extern char **environ; /* Defined by libc */

/* Misc constants */
#define MAXLINE  8192  /* Max text line length */
#define MAXBUF   8192  /* Max I/O buffer size */
#define LISTENQ  1024  /* Second argument to listen() */