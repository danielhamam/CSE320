// Don't make PBX.C File yet, put this code back after you're done with server.c

#include <stdlib.h>
#include "pbx.h" // holds declared functions
#include "debug.h" // for debug statements
#include "string.h" // for strlen()

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

// "Actual structure definitions LOCAL to  PBX module"
typedef struct pbx {
     struct tu *clientUnits[PBX_MAX_EXTENSIONS];
} PBX;

typedef struct tu {
     int clientExtension; // extension # = FD
     int clientFD; // use FD to issue response to that Client
     TU_STATE *clientState; // current state of the client
 } TU;

// ********************************************************************************
//                           All PBX Function DEFINITIONS
//  ********************************************************************************

PBX *pbx_init() {
     // pbx = malloc( (sizeof(int) * 2) * sizeof(PBX_MAX_EXTENSIONS)); // Two ints in TU and max extensions
     pbx = malloc(sizeof(PBX));
     return pbx;
 }

void pbx_shutdown(PBX *pbx) {
     // Wait for threads to terminate
     // Shutdown all connections
     // Free Everything
    return;
}

TU *pbx_register(PBX *pbx, int fd) {

    // Making a new TU with this FD
    TU *targetTU = malloc(sizeof(TU *));
    if (targetTU == NULL) return NULL;
    targetTU->clientExtension = fd;
    targetTU->clientFD = fd;
    targetTU->clientState = TU_ON_HOOK;

    if (fd < 3) { free(targetTU); return NULL; }
    debug("Registering....");
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
        debug("Registered");

        // Write "ON HOOK" to file descriptor
        write(fd, tu_state_names[TU_ON_HOOK], strlen(tu_state_names[TU_ON_HOOK]));

        // Write SPACE to file descriptor
        char space[] = " "; write(fd, space, 1);

        // Write in the INTEGER to file descriptor
        char intHolder[10] = "";
        sprintf(intHolder, "%d", fd);
        write(fd, intHolder, strlen(intHolder));

        // Write a \n (new line) to file descriptor
        write(fd, "\n", 1);

        return targetTU;
    }
    else return NULL;
}

int pbx_unregister(PBX *pbx, TU *tu) {
    return 0;
}

int tu_fileno(TU *tu) {
    return 0;
}

int tu_extension(TU *tu) {
    return 0;
}

int tu_pickup(TU *tu) {
    return 0;
}

int tu_hangup(TU *tu) {
    return 0;
}

int tu_dial(TU *tu, int ext) {
    return 0;
}

int tu_chat(TU *tu, char *msg) {
    return 0;
}
