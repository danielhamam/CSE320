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

static void terminate(int status);
int createListenSocket(int port);
void handler_SIGHUP();

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
                portNumber = convertStr2Int(optarg++); // converts char to int
                if (portNumber < 0) exit(EXIT_FAILURE);
                break;
            default:
                exit(EXIT_FAILURE);
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
    struct sigaction newFunctionality;
    newFunctionality.sa_handler = handler_SIGHUP; // jump to terminate as signal handler
    newFunctionality.sa_flags = SA_RESTART;
    sigemptyset(&newFunctionality.sa_mask);
    sigaction(SIGHUP, &newFunctionality, NULL); // newAction = current act, NULL = old act
// ----------------------------------------------------------------------------------------------

    // Third: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function pbx_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.

    // Variables for setting up server socket
    int serverSocketFD, infinite_loop = 1;
    pthread_t threadID;
    struct sockaddr_storage addressClient;
    socklen_t lengthClient = 0;

    serverSocketFD =  open_listenfd(portNumber); // forms endpoint and returns file descriptor
    if (serverSocketFD < 0) exit(EXIT_FAILURE);

    // Infinite loop because terminates when SIGHUP called
    while(infinite_loop) {
        int acceptCheck = accept(serverSocketFD, (SA *) &addressClient, &lengthClient);
        int *connectionFD = malloc(sizeof(int));
        if (acceptCheck < 0) { free(connectionFD); exit(EXIT_FAILURE); } // check if not successful
        else { // if successful
            *connectionFD = acceptCheck;
            int createCheck = pthread_create(&threadID, NULL, pbx_client_service, connectionFD);
            if (createCheck != 0) exit(EXIT_FAILURE);
        }
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
    // debug("SIGHUP Handler Invoked");
    terminate(0); // EXIT_SUCCESS == 0;
}