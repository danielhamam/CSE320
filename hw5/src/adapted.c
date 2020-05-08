/**
 * This file is basically a subset of CSAPP.C. I used it to adapt
 * from functions that were efficient in facilitating client to server connection.
 *
 * NOTE: some functions may be slightly modified from the original files for my own
 * convienence and to fit precisely what I am trying to accomplish.
 *
 */

#include "adapted.h"
#include "debug.h"

// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------

/*******************************
 * Parts of CSAPP.C I used are here
 *******************************/

int open_listenfd(int port)
{
    int listenfd, optval=1;
    struct sockaddr_in serveraddr;

    /* Create a socket descriptor */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    return -1;

    /* Eliminates "Address already in use" error from bind */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
           (const void *)&optval , sizeof(int)) < 0)
    return -1;

    /* Listenfd will be an endpoint for all requests to port
       on any IP address for this host */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)port);
    if (bind(listenfd, (SA *)&serveraddr, sizeof(serveraddr)) < 0)
    return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, LISTENQ) < 0)
    return -1;
    return listenfd;
}

/*******************************
 * Other Helper Functions I made
 *******************************/

int convertStr2Int(char *message) {
    int holdingInteger = 0;
    while (*message != '\0') {
        if (*message < '0' || *message > '9') { debug("Yes, exit"); return -1; }
        int newValue = *message - 48;
        holdingInteger *= 10;
        holdingInteger += newValue;
        message++;
    }
    // debug("Integer: %d", holdingInteger);
    return holdingInteger;
}