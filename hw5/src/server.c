
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

int *endofmessage;

void *pbx_client_service(void *arg) {

    endofmessage = malloc(sizeof(int));
    *endofmessage = 0;

    // Manipulating the File Descriptor
    int *communicateFD_addr = (int *) arg; // convert argument to INT file descriptor
    int communicateFD = *communicateFD_addr;
    free(communicateFD_addr); // Memory held by FD freed (since we have it now)

    signal(SIGPIPE, SIG_IGN); // Was receiving SIGPIPE in valgrind.

    // Detach thread so I don't need to reap
    int detachCheck = pthread_detach(pthread_self()); // pthread_self "Returns ID of calling thread"
    if (detachCheck != 0) exit(EXIT_FAILURE);

    // "Register client FD with PBX module"
    TU *targetTU = pbx_register(pbx, communicateFD);
    FILE *communicateFilePtr = fdopen(communicateFD, "r"); // opened file descriptor

    // char *messages[50];

    // Now, thread enters so-called "Service Loop"
    int infinite_loop = 1;
    while (infinite_loop) {
        char *receivedCommand = readMsg_Command(communicateFilePtr);
        char *receivedRest = readMsg_afterCommand(receivedCommand, communicateFilePtr);
        processRequest(receivedCommand, receivedRest, targetTU);
        // if (processCheck == -1) continue;
        fflush(communicateFilePtr);
        *endofmessage = 0;
    }
    fclose(communicateFilePtr);
    pbx_unregister(pbx, targetTU);
    // debug("Unregistering");
    free(endofmessage);
    return NULL; // @return is NULL
}

char *readMsg_Command(FILE *communicateFILE) {

    // debug("Step 1: Reading command");

    char *received_CMD = malloc(sizeof(char) * 300); // just to hold whatever message

    // Initialize variables for the loop:
    char *tempMessage = received_CMD;
    unsigned int byte = 0;
    int loopCount = 0;
    // char emptyChar = ' '; // there would be a space between command and referenced message.

    while (byte != '\r' && byte != ' ') {
        byte = fgetc(communicateFILE);
        // debug("BYTE : %c ", (char) byte);
        if (byte == '\n') continue;
        if (byte == '\r' || byte == ' ') break;
        if (byte == EOF) { exit(EXIT_FAILURE); }; // it's saying EOF before "\r\n".
        *tempMessage = byte;
        tempMessage++;
        // debug("Input: %c", byte);
        loopCount++;
    }
    if (byte == '\r') {
        // debug("emptied");
        fgetc(communicateFILE);
        *endofmessage = 1;
    }
    // *tempMessage = '\0';
    received_CMD = realloc(received_CMD,loopCount + 1); // re-allocate to right size

    // debug("The read command is: %s!", received_CMD);
    return received_CMD;
}

char *readMsg_afterCommand(char *receivedCommand, FILE *communicateFILE) {

    if (strncmp(receivedCommand, "pickup", 6) == 0 || strncmp(receivedCommand, "hangup", 6) == 0 ) return NULL;
    if (*endofmessage == 1) return NULL;

    char *receivedMessage = malloc(sizeof(char) * 300); // just to hold whatever message

    int byte = 0;
    int loopCount = 0;
    char *tempMessage = receivedMessage;

    while (byte != '\r') {
        byte = fgetc(communicateFILE);
        if (byte == '\n' && loopCount == 0) { free(receivedMessage); return NULL; }
        if (byte == '\r' || byte == '\n') break;
        if (byte == EOF) exit(EXIT_FAILURE);
        // debug("Adding %c to the string ", *tempMessage);
        loopCount++;
        *tempMessage = byte;
        tempMessage++;
    }
    // debug("The message is %s!", receivedMessage);
    char *heremessage = receivedMessage;
    heremessage++;
    // debug("After char is: %c!", *heremessage);
    *tempMessage = '\0';
    if (byte == '\r') fgetc(communicateFILE); // finish off with \n

    receivedMessage = realloc(receivedMessage, loopCount + 1);
    return receivedMessage;
}

int processRequest(char *receivedCMD, char *received_afterCMD, TU *targetTU) {

    int successful = 0;

    if (strncmp(receivedCMD, "pickup", 6) == 0) { tu_pickup(targetTU); successful = 1; }
    if (strncmp(receivedCMD, "hangup", 6) == 0) { tu_hangup(targetTU); successful = 1; }
    if (strncmp(receivedCMD, "dial", 4) == 0) {
        if (received_afterCMD != NULL) {
            int result = atoi(received_afterCMD);
            tu_dial(targetTU, result);
            // debug("THE NUMBER IS: %d!", result);
            successful = 1; }}
    if (strncmp(receivedCMD, "chat", 4) == 0) { tu_chat(targetTU, received_afterCMD); successful = 1; }

    if (successful == 1) {
        free(receivedCMD);
        if (received_afterCMD != NULL) free(received_afterCMD);
        return 0;
    }
    else {
        return -1;
    }
}