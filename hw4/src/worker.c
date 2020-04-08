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

    signal(SIGSTOP, stop_itself); // else, keep going

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

    struct problem *read_problem;
    int count = 0;

    // Go byte by byte and read the problem
    while (count < sizeof(struct problem)) {
        int byte = fgetc(stdout);
        count++;
    }

    return read_problem;
}

/* WRITING THE PROBLEM TO OUTPUT STREAM*/
void writeResult(struct problem *selectedProblem) {

}