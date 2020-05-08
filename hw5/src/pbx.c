// Don't make PBX.C File yet, put this code back after you're done with server.c

#include <stdlib.h>
#include <sys/socket.h>
#include <semaphore.h> // for mulithreading waits
#include "pbx.h" // holds declared functions
#include "debug.h" // for debug statements
#include "string.h" // for strlen()
#include "adapted.h" // exract parts of CSAPP (waitingP and postingV)

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

void writeStatetoTU(TU *tu); // helper function to write to TU's state to ITSELF
void writeStatetoFD(TU *tu, int fd); // helper function to write to a FILE DESCRIPTOR of the referenced TU's state.

// "Actual structure definitions LOCAL to  PBX module"
typedef struct pbx {
     struct tu *clientUnits[PBX_MAX_EXTENSIONS];
} PBX;

typedef struct tu {
     int clientExtension; // extension # = FD
     int clientFD; // use FD to issue response to that Client
     int requestingTU_FD;
     TU *connected_ringing_PeerTU;
     TU_STATE clientState; // current state of the client
 } TU;

 pthread_mutex_t modularMutex;
 int semCounter;

// ********************************************************************************
//                           All PBX Function DEFINITIONS
//  ********************************************************************************

PBX *pbx_init() {
     pthread_mutex_init(&modularMutex, 0);
     pbx = malloc(sizeof(PBX));  // basically allocating max extensions
     if (pbx == NULL) exit(EXIT_FAILURE);
     for (int i = 0; i < PBX_MAX_EXTENSIONS; i++) pbx->clientUnits[i] = NULL;
     return pbx;
 }

void pbx_shutdown(PBX *pbx) {
     // Wait for threads to terminate
     // Shutdown all connections
     // Free Everything

    pthread_mutex_lock(&modularMutex);

    // Wait for everthing to unregister
    for (int i = 0; i < PBX_MAX_EXTENSIONS; i++) {
        TU *currentTU = pbx->clientUnits[i];
        shutdown(currentTU->clientFD, 2); // 2 is SHUT_RDWR
    }

    // Free PBX.
    pthread_mutex_unlock(&modularMutex);
    pthread_mutex_destroy(&modularMutex);
    free(pbx);
    return;
}

TU *pbx_register(PBX *pbx, int fd) {

    pthread_mutex_lock(&modularMutex);

    // Making a new TU with this FD
    TU *targetTU = malloc(sizeof(TU *));
    if (targetTU == NULL) { pthread_mutex_unlock(&modularMutex); return NULL; }
    targetTU->clientExtension = fd;
    targetTU->clientFD = fd;
    targetTU->clientState = TU_ON_HOOK;

    if (fd < 3) { free(targetTU); pthread_mutex_unlock(&modularMutex);return NULL; }
    // debug("Registering....");
    // Search for a free position in TU array
    int searchCount = 0;
    int placedTU = 0; // boolean value if palced TargetTU
    while (searchCount < PBX_MAX_EXTENSIONS) {
        // if null, don't use
        TU *foundTU = pbx->clientUnits[searchCount];
        if (foundTU == NULL) { pbx->clientUnits[searchCount] = targetTU; placedTU = 1; break; }
        searchCount++;
    }
    if (placedTU == 1) {
        // debug("Registered");
        writeStatetoTU(targetTU);
        pthread_mutex_unlock(&modularMutex);
        return targetTU;
    }
    else {
        pthread_mutex_unlock(&modularMutex);
        return NULL; }
}

int pbx_unregister(PBX *pbx, TU *tu) {

    // waitingP(&modularSemaphore);
    pthread_mutex_lock(&modularMutex);

    // Find this object in the TU Unit array
    int removeCount = 0;
    int foundRemoveTU = 0;
    while (removeCount < PBX_MAX_EXTENSIONS) {
        // break when you find NULL
        TU *removeTU = pbx->clientUnits[removeCount];
        if (removeTU == NULL) { pthread_mutex_unlock(&modularMutex); return -1; } // TU wasn't found (1)
        if (removeTU == tu) { foundRemoveTU = 1; break; }
        removeCount++;
    }
    if (foundRemoveTU == 1) {
        pbx->clientUnits[removeCount] = NULL; // Remove from array list
        free(tu); // Object is "freed"
        pthread_mutex_unlock(&modularMutex);
        return 0;
    }
    pthread_mutex_unlock(&modularMutex);
    return -1; // TU wasn't found (2)
}

int tu_fileno(TU *tu) {

    // waitingP(&modularSemaphore);
    pthread_mutex_lock(&modularMutex);

    int returnFD = tu->clientFD;
    if (tu == NULL || returnFD < 4) { pthread_mutex_unlock(&modularMutex); return -1; } // FD 1 2 3 RESERVED
    // postingV(&modularSemaphore);
    pthread_mutex_unlock(&modularMutex);
    return returnFD;
}

int tu_extension(TU *tu) {

    // waitingP(&modularSemaphore);
    pthread_mutex_lock(&modularMutex);

    int returnExtension = tu->clientExtension;
    if (tu == NULL || returnExtension < 4) { pthread_mutex_unlock(&modularMutex); return -1; } // Tu Units start at extension 4
    // postingV(&modularSemaphore);
    pthread_mutex_unlock(&modularMutex);
    return 0;
}

int tu_pickup(TU *tu) {

    pthread_mutex_lock(&modularMutex);
    // debug("Waiting P ==> PICKUP");

    if (tu == NULL) { pthread_mutex_unlock(&modularMutex); return -1; }

    // Case 1: ON HOOK --> DIAL TONE
    if (tu->clientState == TU_ON_HOOK) {
        tu->clientState = TU_DIAL_TONE;
        writeStatetoTU(tu);
        pthread_mutex_unlock(&modularMutex);
        return 0;
    }
    // Case 2: RINGING --> CONNECTED
    else if (tu->clientState == TU_RINGING) {

        // Transition to Connected
        tu->clientState = TU_CONNECTED;
        writeStatetoTU(tu);

        // Calling TU also transitions to Connected
        TU *dialingTU = tu->connected_ringing_PeerTU;
        dialingTU->clientState = TU_CONNECTED;
        dialingTU->connected_ringing_PeerTU = tu;
        dialingTU->requestingTU_FD = tu->clientFD;
        writeStatetoTU(dialingTU);
        pthread_mutex_unlock(&modularMutex);
        return 0;
    }
    else { writeStatetoTU(tu); pthread_mutex_unlock(&modularMutex); return 0; }
}

int tu_hangup(TU *tu) {

    // waitingP(&modularSemaphore);
    pthread_mutex_lock(&modularMutex);
    // debug("Waiting P ==> HANGUP");

    if (tu == NULL) { pthread_mutex_unlock(&modularMutex); return -1; }

    // Case 1: CONNECTED --> ON_HOOK
    if (tu->clientState == TU_CONNECTED) {
        tu->clientState = TU_ON_HOOK;
        writeStatetoTU(tu);

        // Change peer TU's state to Dial Tone
        TU *peerTU = tu->connected_ringing_PeerTU;
        peerTU->clientState = TU_DIAL_TONE;
        writeStatetoTU(peerTU);

    }
    // Case 2: RINGING_BACK --> ON_HOOK
    else if (tu->clientState == TU_RING_BACK) {
        tu->clientState = TU_ON_HOOK;
        writeStatetoTU(tu);

        // Change peer TU's state to ON HOOK
        TU *peerTU = tu->connected_ringing_PeerTU;
        peerTU->clientState = TU_ON_HOOK;
        writeStatetoTU(peerTU);
    }
    // Case 3: RINGING --> ON HOOK
    else if (tu->clientState == TU_RINGING) {
        tu->clientState = TU_ON_HOOK;
        writeStatetoTU(tu);

        // Change peer TU's state to Dial Tone
        TU *peerTU = tu->connected_ringing_PeerTU;
        peerTU->clientState = TU_DIAL_TONE;
        writeStatetoTU(peerTU);
    }
    else if (tu->clientState == TU_DIAL_TONE || tu->clientState == TU_BUSY_SIGNAL || tu->clientState == TU_ERROR) {
        tu->clientState = TU_ON_HOOK;
        writeStatetoTU(tu);
    }
    else writeStatetoTU(tu);
    // postingV(&modularSemaphore);
    pthread_mutex_unlock(&modularMutex);
    return 0;
}

int tu_dial(TU *tu, int ext) {

    // waitingP(&modularSemaphore);
    pthread_mutex_lock(&modularMutex);
    // debug("Waiting P ==> DIAL");

    if (tu == NULL) { pthread_mutex_unlock(&modularMutex); return -1; }
    if (ext < 4) { pthread_mutex_unlock(&modularMutex); return -1; }

    if (tu->clientState != TU_DIAL_TONE) {
        writeStatetoTU(tu);
        // postingV(&modularSemaphore);
        pthread_mutex_unlock(&modularMutex);
        return 0;
    }

    // Variables
    int noneFound = 1; // 1 if NO TU WITH THAT EXTENSION FOUND
    TU *searchedTU = NULL;

    // Check if extension is valid:
    for (int i = 0; i < PBX_MAX_EXTENSIONS; i++) {
        if (pbx->clientUnits[i] == NULL) continue;
        searchedTU = pbx->clientUnits[i];
        if (searchedTU->clientExtension == ext) { noneFound = 0; break; } // we found one
    }

    // If none found, ERROR transition
    if (noneFound == 1) {
        tu->clientState = TU_ERROR;
        writeStatetoTU(tu);
        // postingV(&modularSemaphore);
        pthread_mutex_unlock(&modularMutex);
        return 0;
    }
    TU *dialedTU = searchedTU;
    // If found and TU was in TU_DIAL_TONE state
    if (noneFound == 0 && tu->clientState == TU_DIAL_TONE) {

        if (ext == tu->clientFD) { // it's the same number, and it's DIAL_TONE
            tu->clientState = TU_BUSY_SIGNAL;
            writeStatetoTU(tu);
            // postingV(&modularSemaphore);
            pthread_mutex_unlock(&modularMutex);
            return 0;
        }

        if (dialedTU->clientState == TU_ON_HOOK) {
            // Calling TU transitions to Ring Back
            tu->clientState = TU_RING_BACK;
            tu->connected_ringing_PeerTU = dialedTU;
            writeStatetoTU(tu);

            // Dialed TU transitions to RINGING
            dialedTU->clientState = TU_RINGING;
            dialedTU->requestingTU_FD = tu->clientFD; // This is the FD of the TU that's calling
            dialedTU->connected_ringing_PeerTU = tu;
            writeStatetoTU(dialedTU); // "extension also notified of its new state"
        }
        else {
            // If not ON HOOK, calling TU to BUSY SIGNAL (no change to dialed extension)
            tu->clientState = TU_BUSY_SIGNAL;
            writeStatetoTU(tu); // write state it self
        }
    }
    else writeStatetoTU(tu); // no state change
    // postingV(&modularSemaphore);
    pthread_mutex_unlock(&modularMutex);
    return 0;
}

int tu_chat(TU *tu, char *msg) {

    pthread_mutex_lock(&modularMutex);
    // debug("Waiting P ==> CHATS");

    if (tu == NULL) { return -1; }

    if (msg == NULL) {
        writeStatetoTU(tu);
        pthread_mutex_unlock(&modularMutex);
        return -1;
    }

    if(tu->clientState != TU_CONNECTED) {
        writeStatetoTU(tu);
        pthread_mutex_unlock(&modularMutex);
        return -1;
    }

    // Assume it's connected, now send messages
    TU *peerTU = tu->connected_ringing_PeerTU;
    char premsg[] = "CHAT ";
    write(peerTU->clientFD, premsg, strlen(premsg));
    write(peerTU->clientFD, msg, strlen(msg));
    write(peerTU->clientFD, "\n", 1);

    // Print out current connection state
    writeStatetoTU(tu);
    pthread_mutex_unlock(&modularMutex);
    return 0;
}

/*
*
* writeStatetoTU: writes the @param TU's State to its file descriptor
* @param: TU pointer of which client state should be printed
* @return: void
*/

void writeStatetoTU(TU *tu) {

    write(tu->clientFD, tu_state_names[tu->clientState], strlen(tu_state_names[tu->clientState])); // Writing String of Command to the FD

    if (tu->clientState == TU_CONNECTED) {

        char space[] = " "; write(tu->clientFD, space, 1); // Write SPACE to file descriptor
        char intHolder[10] = "";  // Write in the INTEGER to file descriptor
        sprintf(intHolder, "%d", tu->requestingTU_FD);
        write(tu->clientFD, intHolder, strlen(intHolder));
    }
    else if (tu->clientState == TU_ON_HOOK) {

        char space[] = " "; write(tu->clientFD, space, 1); // Write SPACE to file descriptor
        char intHolder[10] = "";  // Write in the INTEGER to file descriptor
        sprintf(intHolder, "%d", tu->clientFD);
        write(tu->clientFD, intHolder, strlen(intHolder));
    }
    write(tu->clientFD, "\n", 1); // Write a \n (new line) to file descriptor
}

/*
*
* writeStatetoFD: writes the @param TU's State to a referenced file descriptor
* @param: TU pointer of which client state should be printed
* @param: File descriptor of a TU of which the referenced tu's client state should be written to
*/

void writeStatetoFD(TU *tu, int fd) {

    write(fd, tu_state_names[tu->clientState], strlen(tu_state_names[tu->clientState])); // Write "ON HOOK" to file descriptor
    if (tu->clientState == TU_ON_HOOK || tu->clientState == TU_CONNECTED) {
        char space[] = " "; write(fd, space, 1); // Write SPACE to file descriptor
        char intHolder[10] = "";  // Write in the INTEGER to file descriptor
        sprintf(intHolder, "%d", fd);
        write(fd, intHolder, strlen(intHolder));
    }
    write(fd, "\n", 1); // Write a \n (new line) to file descriptor
    return;
}