#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "pbx.h"
#include "server.h"
#include "debug.h"
#include "adapted.h"
// #include "csapp.c"

static void terminate(int status);
int createListenSocket(int port);

/*
 * "PBX" telephone exchange simulation.
 *
 * Usage: pbx <port>
 */
int main(int argc, char* argv[]){

    // First: Option processing should be performed here. Option '-p <port>' is
    // required in order to specify the port number on which the server should listen.

    int flag, portNumber;

    while((flag = getopt(argc, argv, "p:")) != EOF) {

        switch(flag) {
            case 'p':
                portNumber = atoi(optarg++); // converts char to int
                if (portNumber < 0) exit(EXIT_FAILURE);
                break;
        } // end of switch
    } // end of while loop
// ----------------------------------------------------------------------------------------------
    debug("Port number: %d ", portNumber);
    // Perform required initialization of the PBX module.
    debug("Initializing PBX...");
    pbx = pbx_init();

// ----------------------------------------------------------------------------------------------
    // Second: Set Signal Handler for "SIGHUP" --> cleanly terminate
    struct sigaction newAction;
    newAction.sa_handler = terminate; // jump to terminate as signal handler
    // newAction.sa_flags = SA_RESTART; /* Restart syscalls if possible */ // got from csapp.c
    sigaction(SIGHUP, &newAction, NULL); // newAction = current act, NULL = old act
// ----------------------------------------------------------------------------------------------

    // Third: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function pbx_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.

    int serverSocketFD, infinite_loop = 1;
    serverSocketFD =  open_listenfd(portNumber); // forms endpoint and returns file descriptor
    // if (serverSocketFD < 0) exit(EXIT_FAILURE);
    debug("SERVER: %d ", serverSocketFD);
    terminate(0);

    // Infinite loop because terminates when SIGHUP called
    while(infinite_loop) {

    }

}

/*
 * Function called to cleanly shut down the server.
 */
void terminate(int status) {
    debug("Shutting down PBX...");
    pbx_shutdown(pbx);
    debug("PBX server terminating");
    exit(status);
}

/*
 * SIGHUP Handler to gracefully terminate the server
 */

void handler_SIGHUP() {
    debug("SIGHUP Handler Invoked");
    terminate(0); // EXIT_SUCCESS == 0;
}

/*
 * Function called to open/return LISTENING socket on requested port
 */