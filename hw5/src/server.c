
#include "adapted.h" // holds all the libraries i want to include
#include "pbx.h" // to include pbx_register, and pbx global variable

/**
 * TASK II: Server Module
 *
 * This file implements the server module which includes the function pbx_client_service(), used when client connects
 * to a server.
 *
 */

// Helper functions
char *readMsg_Command(int fileDesc);
char *readMsg_afterCommand(char *receivedCommand, int fileDesc);
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

    // Now, thread enters so-called "Service Loop"
    int infinite_loop = 1;
    while (infinite_loop) {
        char *receivedCommand = readMsg_Command(communicateFD);
        char *receivedRest = readMsg_afterCommand(receivedCommand, communicateFD);
        processRequest(receivedCommand, receivedRest, targetTU);
    }
    return NULL; // @return is NULL
}

char *readMsg_Command(int fileDesc) {

    FILE *communicateFILE = fdopen(fileDesc, "r"); // opened file descriptor

    char *received_CMD = malloc(sizeof(char) * 300); // just to hold whatever message

    // Initialize variables for the loop:
    char *tempMessage = received_CMD;
    int byte = 0;
    int loopCount = 0;
    char emptyChar = ' ';

    while (byte != emptyChar) {
        unsigned int byte = fgetc(communicateFILE);
        if (byte == EOF) exit(EXIT_FAILURE); // it's saying EOF before "\r\n".
        *tempMessage = byte;
        tempMessage++;
        loopCount++;
    }

    received_CMD = realloc(received_CMD,loopCount); // re-allocate to right size
    return received_CMD;
}

char *readMsg_afterCommand(char *receivedCommand, int fileDesc) {

    if (strcmp(receivedCommand, "pickup") == 0 || strcmp(receivedCommand, "hangup") == 0 ) return " ";

    FILE *communicateFILE = fdopen(fileDesc, "r"); // opened file descriptor
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
    return receivedMessage;
}

int processRequest(char *receivedCMD, char *received_afterCMD, TU *targetTU) {

    if (strcmp(receivedCMD, "pickup") == 0) { tu_pickup(targetTU); return 0; }
    if (strcmp(receivedCMD, "hangup") == 0) { tu_hangup(targetTU); return 0; }
    if (strcmp(receivedCMD, "dial") == 0) { tu_dial(targetTU, convertStr2Int(received_afterCMD)); return 0; }
    if (strcmp(receivedCMD, "chat") == 0) { tu_chat(targetTU, received_afterCMD); return 0; }

    return -1;

}

int convertStr2Int(char *message) {
    int holdingInteger = 0;
    while (*message != '\0') {
        if (*message <= '0' || *message >= '9') return -1;
        int newValue = *message - 48;
        holdingInteger = holdingInteger * 10;
        holdingInteger = holdingInteger + newValue;
        message++;
    }
    return holdingInteger;
}