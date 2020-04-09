#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "polya.h"

/*
 * worker
 * (See polya.h for specification.)
 */

// Signal Handlers, don't do anything "compilicated"
void stop_itself(); // fort worker process to stop itself
void sighup_handler();
void sigterm_handler();


struct problem *readProblem();
void writeResult(struct result *selectedResult, FILE *out);

int worker(void) {

    // TO BE IMPLEMENTED
    // Work on this first and use demo/polya for testing.

    // Worker Process installs handlers for SIGHUP and SIGTERM
    signal(SIGHUP, sighup_handler);
    signal(SIGTERM, sigterm_handler);

    // continues until find solution, fails to find, or SIGHUP by master to cancel
    // worker reads problems from standard input and writes results to standard output
    struct problem *selectProblem = readProblem();

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

    // Initialize new pointer for problem
    struct problem *read_problem = NULL;

    // Go byte by byte and read the problem

    // First, read the size variable
    int count_size = 0;
    int tempSize = 0;
    while (count_size < sizeof(size_t)) {
        int byte = fgetc(stdin);
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
    int sizeArrays = (int) read_problem->size - sizeof(struct problem);

    char *array = (char *) malloc(sizeArrays); // We allocated for the amount of padding/data
    memcpy(read_problem->data, &array, sizeArrays); // Copy from the array we made to the read_problem->data)

    return read_problem;
}

/* WRITING THE PROBLEM TO OUTPUT STREAM*/
void writeResult(struct result *selectedResult, FILE *out) {

    char *memHolder = NULL;
    memcpy(&memHolder, &selectedResult, sizeof(*selectedResult));

    while (memHolder != NULL) {
        fputc(*memHolder, out);
        memHolder++;
    }
}

void stop_itself(void) {}

void sighup_handler(void) {}

void sigterm_handler(void) {

    // Graceful termination of worker, use exit()
    exit(2);

}