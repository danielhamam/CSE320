#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"
#include "polya.h"

/*
 * worker
 * (See polya.h for specification.)
 */

// Signal Handlers, don't do anything "compilicated"
void stop_itself(); // fort worker process to stop itself
void SIGHUP_handler();
void SIGTERM_handler();
void SIGCONT_handler();

struct problem *readProblem();
void writeResult(struct result *selectedResult, FILE *out);

volatile sig_atomic_t *CHECK_FLAG = 0; // 0 = Normal Function
int flag_succeeded = 0; // 1 for success (for SIGHUP)

int worker(void) {
    // Work on this first and use demo/polya for testing.

    // So-called initialiation
    signal(SIGHUP, SIGHUP_handler);
    signal(SIGTERM, SIGTERM_handler);

    // Main (infinity) loop for reading from master process: (continues till SIGTERM)
    while (1) {
        kill(getpid(), 19); // SEND ITSELF SIGSTOP, AWAITS CONTINUE BY MASTER. (becomes idle when SIGSTOP SENDS)
        struct problem *targetProblem = readProblem(stdin);
        debug("read_problem->id %d ", (int) targetProblem->id);
        debug("read_problem->type %d ", (int) targetProblem->type);
        struct solver_methods targetMethod = solvers[targetProblem->type]; // "used to invoke proper solver for each problem"
        struct result *targetRESULT = targetMethod.solve(targetProblem,CHECK_FLAG);
        writeResult(targetRESULT, stdout);
        // free what you malloced
        free(targetProblem);
        free(targetRESULT); // malloced in solver, so free now
    }

    return EXIT_FAILURE;
}

// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
//                            HELPER FUNCTIONS
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------


/* READING THE PROBLEM FROM INPUT STREAM*/
// TAKES NOTHING, RETURNS READ PROBLEM
struct problem *readProblem(FILE *stream) {
    debug("Reading the new problem");
    // Initialize new pointer for problem
    struct problem *read_problem = malloc(sizeof(*read_problem));

    // Go byte by byte and read the problem

    // First, read the size variable
    int count_size = 0;
    int tempSize = 0;
    while (count_size < sizeof(size_t)) {
        int byte = fgetc(stream);
        if (byte == EOF) exit(EXIT_FAILURE);
        tempSize += byte;
        count_size++;
    }
    read_problem->size = (size_t) tempSize;

    // Second, read the short type variable
    int count_type = 0;
    int tempType = 0;
    while(count_type < sizeof(short)) {
        int byte = fgetc(stream);
        if (byte == EOF) exit(EXIT_FAILURE);
        tempType += byte;
        count_type++;
    }
    read_problem->type = (short) tempType;

    // Third, read the short ID variable
    int count_ID = 0;
    int tempID = 0;
    while (count_ID < sizeof(short)) {
        int byte = fgetc(stream);
        if (byte == EOF) exit(EXIT_FAILURE);
        tempID += byte;
        count_ID++;
    }
    read_problem->id = (short) tempID;

    // Fourth, read the short nvars variable
    int count_nvars = 0;
    int tempNVARS = 0;
    while (count_nvars < sizeof(short)) {
        int byte = fgetc(stream);
        if (byte == EOF) exit(EXIT_FAILURE);
        tempNVARS += byte;
        count_nvars++;
    }
    read_problem->nvars = (short) tempNVARS;

    // Fifth, read the short var variable
    int count_var = 0;
    int tempVar = 0;
    while (count_var < sizeof(short)) {
        int byte = fgetc(stream);
        if (byte == EOF) exit(EXIT_FAILURE);
        tempVar += byte;
        count_var++;
    }
    read_problem->var = (short) tempVar;

    // Sixth, read the char padding[0] variable
    // Seventh, read the char data[0] variable
    int sizeArrays = (int) (read_problem->size) - sizeof(struct problem);

    char *array = (char *) malloc(sizeArrays); // We allocated for the amount of padding/data
    memcpy(read_problem->data, &array, sizeArrays); // Copy from the array we made to the read_problem->data)

    return read_problem;
}

/* WRITING THE PROBLEM TO OUTPUT STREAM*/
void writeResult(struct result *selectedResult, FILE *out) {

    debug("Writing the result from the worker process");
    debug("RESULT SIZE --> %d ", (int) selectedResult->size);
    debug("ID: %d ", selectedResult->id);
    debug("FAILED: %d ", selectedResult->failed);

    char *charPtr = (char *) selectedResult;
    while (charPtr != NULL) {
        fputc(*charPtr, out);
        charPtr++;
    }

    // int countSize = 0;
    // int shiftNum = 0;
    // while (countSize < sizeof(size_t)) {
    //     int byte = (int) (selectedResult->size >> shiftNum) & 0xFF; // 8 bits in a byte
    //     fputc(byte, out);
    //     countSize++;
    //     shiftNum += 8;
    // }
    // int countID = 0;
    // int shiftNum2 = 0;
    // while (countID < sizeof(short)) {
    //     int byte = (int) (selectedResult->id >> shiftNum2) & 0xFF; // 8 bits in a byte
    //     fputc(byte, out);
    //     countID++;
    //     shiftNum2 += 8;
    // }

    // // Failed is just one byte
    // fputc(selectedResult->failed, out);

    // // Rest are char arrays
    // fputs(selectedResult->padding, out);
    // fputs(selectedResult->data, out);

    fflush(out);
}

void stop_itself(void) {}

void SIGHUP_handler(void) {
    // Write result to pipe (might or might not be "failed") --> check if already succeeded
    // cancel its current solution attempt
    *CHECK_FLAG = 1;
    debug("SIGHUP Handler invoked");

    kill(getpid(), 19); // SEND ITSELF SIGSTOP
}

void SIGTERM_handler(void) {
    // Graceful termination of worker, use exit()
    debug("SIGTERM_handler invoked");
    *CHECK_FLAG = -1; // so cancels solution when trying to solve
    exit(EXIT_SUCCESS);
}