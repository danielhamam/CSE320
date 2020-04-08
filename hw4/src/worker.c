#include <stdlib.h>

#include "debug.h"
#include "polya.h"

/*
 * worker
 * (See polya.h for specification.)
 */

void stop_itself(); // fort worker process to stop itself
struct problem *readProblem();
void writeResult(struct problem *selectedProblem);

int worker(void) {

    // TO BE IMPLEMENTED
    // Work on this first and use demo/polya for testing.

    // continues until find solution, fails to find, or SIGHUP by master to cancel
    // worker reads problems from standard input and writes results to standard output
    readProblem();

    return EXIT_FAILURE;
}

// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
//                            HELPER FUNCTIONS
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------

void stop_itself(void) {}


/* READING THE PROBLEM FROM INPUT STREAM*/
// TAKES NOTHING, RETURNS READ PROBLEM
struct problem *readProblem() {

    // Initialize new pointer for problem
    struct problem *read_problem;
    // read_problem->size = (size_t) 0;
    // read_problem->type = (short) 0;

    // Go byte by byte and read the problem

    // First, read the size variable
    int count_size = 0;
    int tempSize = 0;
    while (count_size < sizeof(struct problem)) {
        int byte = fgetc(stdin);
        tempSize += byte;
        count_size++;
    }
    read_problem->size = (size_t) tempSize;

    // Second, read the short type variable
    int count_type = 0;
    int tempType = 0;
    while(count_type < sizeof(short)) {
        int byte = fgetc(stdin);
        tempType += byte;
        count_type++;
    }
    read_problem->type = (short) tempType;

    // Third, read the short ID variable
    int count_ID = 0;
    int tempID = 0;
    while (count_ID < sizeof(short)) {
        int byte = fgetc(stdin);
        tempID += byte;
        count_ID++;
    }
    read_problem->id = (short) tempID;

    // Fourth, read the short nvars variable
    int count_nvars = 0;
    int tempNVARS = 0;
    while (count_nvars < sizeof(short)) {
        int byte = fgetc(stdin);
        tempNVARS += byte;
        count_nvars++;
    }
    read_problem->nvars = (short) tempNVARS;

    // Fifth, read the short var variable
    int count_var = 0;
    int tempVar = 0;
    while (count_var < sizeof(short)) {
        int byte = fgetc(stdin);
        tempVar += byte;
        count_var++;
    }
    read_problem->var = (short) tempVar;

    // Sixth, read the char padding[0] variable
    // Seventh, read the char data[0] variable


    return read_problem;
}

/* WRITING THE PROBLEM TO OUTPUT STREAM*/
void writeResult(struct problem *selectedProblem) {}