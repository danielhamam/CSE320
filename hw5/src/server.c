
#include "adapted.h" // holds all the libraries i want to include
#include "pbx.h" // to include pbx_register, and pbx global variable
#include "debug.h"

/**
 * TASK II: Server Module
 *
 * This file implements the server module which includes the function pbx_client_service(), used when client connects
 * to a server.
 *
 */

// Helper functions
char *readMsg_Command(FILE *communicateFilePtr);
char *readMsg_afterCommand(char *receivedCommand, FILE *communicateFILE);
int processRequest(char *receivedCMD, char *received_afterCMD, TU *target);
int convertStr2Int(char *message);

void *pbx_client_service(void *arg) {

    // Manipulating the File Descriptor
    int *communicateFD_addr = (int *) arg; // convert argument to INT file descriptor
    int communicateFD = *communicateFD_addr;
    free(communicateFD_addr); // Memory held by FD freed (since we have it now)

    // Detach thread so I don't need to reap
    int detachCheck = pthread_detach(pthread_self()); // pthread_self "Returns ID of calling thread"
    if (detachCheck != 0) exit(EXIT_FAILURE);

    // "Register client FD with PBX module"
    TU *targetTU = pbx_register(pbx, communicateFD);
    FILE *communicateFilePtr = fdopen(communicateFD, "r"); // opened file descriptor

    // Now, thread enters so-called "Service Loop"
    int infinite_loop = 1;
    while (infinite_loop) {
        char *receivedCommand = readMsg_Command(communicateFilePtr);
        char *receivedRest = readMsg_afterCommand(receivedCommand, communicateFilePtr);
        int processCheck = processRequest(receivedCommand, receivedRest, targetTU);
        if (processCheck == -1) exit(EXIT_FAILURE);
        fflush(communicateFilePtr);
    }
    return NULL; // @return is NULL
}

char *readMsg_Command(FILE *communicateFILE) {

    // debug("Step 1: Reading command");

    char *received_CMD = malloc(sizeof(char) * 300); // just to hold whatever message

    // Initialize variables for the loop:
    char *tempMessage = received_CMD;
    int byte = 0;
    int loopCount = 0;
    // char emptyChar = ' '; // there would be a space between command and referenced message.

    while (byte != '\r' && byte != ' ') {
        unsigned int byte = fgetc(communicateFILE);
        if (byte == '\n') continue;
        if (byte == '\r' || byte == ' ') break;
        if (byte == EOF) { exit(EXIT_FAILURE); }; // it's saying EOF before "\r\n".
        *tempMessage = byte;
        tempMessage++;
        // debug("Input: %c", byte);
        loopCount++;
    }

    *tempMessage = '\0';
    received_CMD = realloc(received_CMD,loopCount + 1); // re-allocate to right size

    debug("The read command is: %s!", received_CMD);
    return received_CMD;
}

char *readMsg_afterCommand(char *receivedCommand, FILE *communicateFILE) {

    if (strncmp(receivedCommand, "pickup", 6) == 0 || strncmp(receivedCommand, "hangup", 6) == 0 ) return " ";

    // debug("Step 2: Reading message after command");

    char *receivedMessage = malloc(sizeof(char) * 300); // just to hold whatever message

    int byte = 0;
    int loopCount = 0;
    char *tempMessage = receivedMessage;

    while (byte != '\r') {
        unsigned int byte = fgetc(communicateFILE);
        if (byte == '\r') break;
        if (byte == EOF) exit(EXIT_FAILURE);
        loopCount++;
        *tempMessage = byte;
        tempMessage++;
    }
    *tempMessage = '\0';
    debug("The message is %s!", receivedMessage);
    receivedMessage = realloc(receivedMessage, loopCount + 1);
    fflush(communicateFILE);
    return receivedMessage;
}

int processRequest(char *receivedCMD, char *received_afterCMD, TU *targetTU) {

    if (strncmp(receivedCMD, "pickup", 6) == 0) { tu_pickup(targetTU); return 0; }
    if (strncmp(receivedCMD, "hangup", 6) == 0) { tu_hangup(targetTU); return 0; }
    if (strncmp(receivedCMD, "dial", 4) == 0) { tu_dial(targetTU, convertStr2Int(received_afterCMD)); return 0; }
    if (strncmp(receivedCMD, "chat", 4) == 0) { tu_chat(targetTU, received_afterCMD); return 0; }
    return -1;
}

// Don't make PBX.C File yet, put this code back after you're done with server.c

// #include <stdlib.h>
// #include "pbx.h" // holds declared functions

/**
 * TASK III: PBX Module
 *
 * This file implements the functions declared in pbx.h. This is basically the central
 * module of the whole program. Holds a registry of connected clients and manages TU units.
 *
 */

/*********************************************************************************
                         Variables/Functions/Structures Declared
 *********************************************************************************/

// // "Actual structure definitions LOCAL to  PBX module"
// struct PBX {
//     TU *clientUnit;
// };

// struct TU {
//     int *clientExtension; // extension # = FD
//     int *clientFD; // use FD to issue response to that Client
// };

// ********************************************************************************
//                           All PBX Function DEFINITIONS
//  ********************************************************************************

// PBX *pbx_init() {
//     pbx = malloc( (sizeof(int) * 2) * sizeof(PBX_MAX_EXTENSIONS)); // Two ints in TU and max extensions
//     return pbx;
// }

// pbx_shutdown(PBX *pbx) {
//     // Wait for threads to terminate
//     // Shutdown all connections
//     // Free Everything
// }