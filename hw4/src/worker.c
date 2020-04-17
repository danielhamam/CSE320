#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"
#include "polya.h"

/*
 * worker
 * (See polya.h for specification.)
 */

// ---------------------------------------------------------------
// Signal Handlers, don't do anything "compilicated"
void SIGHUP_handler();
void SIGTERM_handler();
void SIGCONT_handler();
struct problem *readProblem();
void writeResult(struct result *selectedResult, FILE *out);
volatile sig_atomic_t CHECK_FLAG = 0; // Normal Function
// ---------------------------------------------------------------

int worker(void) {
    // Work on this first and use demo/polya for testing.

    // So-called initialization
    signal(SIGHUP, SIGHUP_handler);
    signal(SIGTERM, SIGTERM_handler);
    // signal(SIGCONT, SIGCONT_handler);
    // sigset_t newMask;
    // Blocking SIGTERM..............
    // sigfillset(&newMask);
    // sigdelset(&newMask, SIGCONT);

    // Main (infinity) loop for reading from master process: (continues till SIGTERM)
    while (1) {
        kill(getpid(), 19); // SEND ITSELF SIGSTOP, AWAITS CONTINUE BY MASTER. (becomes idle when SIGSTOP SENDS)
        // kill(getppid(), 17); // Send SIGCHLD to parent?

// ------------------------------------------------------------
        // sigsuspend(&newMask); // waiting for the SIGCONT signal
        struct problem *targetProblem = readProblem(stdin);
        struct solver_methods targetMethod = solvers[targetProblem->type]; // "used to invoke proper solver for each problem"
        debug("Found targetMethod");
        struct result *targetRESULT = targetMethod.solve(targetProblem, &CHECK_FLAG);
        debug("Found result, before writing");
        writeResult(targetRESULT, stdout);
        // free what you malloced
        // free(targetProblem);
        // free(targetRESULT); // malloced in solver, so free now
// ------------------------------------------------------------

    }

    return EXIT_SUCCESS;
}

// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
//                            HELPER FUNCTIONS
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------


/* READING THE PROBLEM FROM INPUT STREAM*/
// TAKES NOTHING, RETURNS READ PROBLE
struct problem *readProblem(FILE *stream) {
    debug("Reading the new problem");

    // Initialize new pointer for problem
    struct problem *read_problem_temp = (struct problem *) malloc(sizeof(struct problem));

    // Go byte by byte and read the problem

    // First, read the size variable
    size_t count_size = 0;
    unsigned int tempSize = 0;
    while (count_size < sizeof(size_t)) {
        unsigned int byte = fgetc(stream);
        if (byte == EOF) exit(EXIT_FAILURE);
        tempSize += byte;
        count_size++;
    }

    // --------------------------------------------------------------------------------------------------------
    // --------------------------------------------------------------------------------------------------------

    // Re-allocate to the right size

    struct problem *read_problem = (struct problem *) realloc(read_problem_temp, tempSize);
    read_problem->size = (size_t) tempSize;

    // Second, read the short type variable
    int count_type = 0;
    unsigned int tempType = 0;
    while(count_type < sizeof(short)) {
        unsigned int byte = fgetc(stream);
        if (byte == EOF) exit(EXIT_FAILURE);
        tempType += byte;
        count_type++;
    }
    read_problem->type = (short) tempType;

    // Third, read the short ID variable
    int count_ID = 0;
    unsigned int tempID = 0;
    while (count_ID < sizeof(short)) {
        unsigned int byte = fgetc(stream);
        if (byte == EOF) exit(EXIT_FAILURE);
        tempID += byte;
        count_ID++;
    }
    read_problem->id = (short) tempID;

    // Fourth, read the short nvars variable
    int count_nvars = 0;
    unsigned int tempNVARS = 0;
    while (count_nvars < sizeof(short)) {
        unsigned int byte = fgetc(stream);
        if (byte == EOF) exit(EXIT_FAILURE);
        tempNVARS += byte;
        count_nvars++;
    }
    read_problem->nvars = (short) tempNVARS;

    // Fifth, read the short var variable
    int count_var = 0;
    unsigned int tempVar = 0;
    while (count_var < sizeof(short)) {
        unsigned int byte = fgetc(stream);
        if (byte == EOF) exit(EXIT_FAILURE);
        tempVar += byte;
        count_var++;
    }
    read_problem->var = (short) tempVar;

    // Sixth, read the char padding[0] variable
    // void *array1 = malloc(sizeof(char *));
    // memcpy(read_problem->padding, array1, (size_t) sizeof(char *));
    // char *tempPadding = read_problem->data;
    // *tempPadding = (int) fgetc(stream);

    // Seventh, read the char data[0] variable
    int sizeData = (int) (read_problem->size) - sizeof(struct problem);

    void *array2 = malloc(sizeData);
    memcpy(read_problem->data, array2, (size_t) sizeData); // Copy from the array we made to the read_problem->data)
    free(array2);

    // We malloced space for the data, so now actually read it
    // Char pointer points to beginning of data section
    int countData = 0;
    char *tempData = read_problem->data;
    while (countData < sizeData) {
        unsigned int byte = fgetc(stream);
        // debug("BYTE: %d ",byte );
        if (byte == EOF) exit(EXIT_FAILURE);
        *tempData = byte; // store char in this position
        countData++;
        tempData++;
    }

    debug("Problem: size: %ld, type: %d, id: %d, nvars: %d, var :%d ", read_problem->size, read_problem->type, read_problem->id, read_problem->nvars, read_problem->var);
    debug("Problem data: %s ", read_problem->data);

    return read_problem;
}

/* WRITING THE PROBLEM TO OUTPUT STREAM*/
void writeResult(struct result *selectedResult, FILE *out) {

    debug("Writing the result from the worker process");

    char *charPtr = (char *) selectedResult;
    if (charPtr == NULL) return exit(EXIT_FAILURE);
    int countPtr = 0;
    while (countPtr < sizeof(*selectedResult)) {
        if (*charPtr == EOF) return exit(EXIT_FAILURE);
        fputc(*charPtr, out);
        charPtr++;
        countPtr++;
    }
    fflush(out);
}

void SIGHUP_handler(void) {
    // Write result to pipe (might or might not be "failed") --> check if already succeeded
    // cancel its current solution attempt
    CHECK_FLAG = 1;
    debug("SIGHUP Handler invoked");
}

void SIGTERM_handler(void) {
    // Graceful termination of worker, use exit()
    debug("SIGTERM Handler invoked");
    exit(EXIT_SUCCESS);
}

void SIGCONT_handler(void) {
    debug("SIGCONT Handler invoked");
    // *CHECK_FLAG = 0; // reset CHECK_FLAG
    kill(getpid(), 18); // send itself continue signal

}