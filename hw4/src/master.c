#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include "debug.h"
#include "polya.h"

/*
 * master
 * (See polya.h for specification.)
 */

struct problem *generateProblem();
void writeProblem(struct problem *problem, FILE *stream);

int master(int workers) {

    sf_start();

//                           INITIALIZE THE PROGRAM
// ------------------------------------------------------------------------

    pid_t arrayPID[workers]; // i dont think you have to malloc because only exist in this code block

    int masterToworker_pipes[workers][2]; // ARRAY (PIPE DESCRIPTORS), MASTER SENDS TO WORKER
    int workerTomaster_pipes[workers][2]; // ARRAY (PIPE DESCRIPTORS), WORKER SENDS TO MASTER

    // Create pipes for every single worker
    int count = 0;
    while (count < workers) {
        if (pipe(masterToworker_pipes[count]) == -1) exit(EXIT_FAILURE);
        if (pipe(workerTomaster_pipes[count]) == -1) exit(EXIT_FAILURE);

        // Redirect standard input/output to pipes ( pipe[0] = read end of the pipe, pipe[1] = write end of the pipe )
        // In unistd.h, 0 is file descriptor for standard input, 1 for standard output, 2 standard error output
        dup2(0, masterToworker_pipes[count][0]); // move pipe into stdin (WRITING END)
        close(masterToworker_pipes[count][0]); // close READING END of pipe

        dup2(1, workerTomaster_pipes[count][1]); // move pipe into stdout (WRITING END), write worker to masker
        close(workerTomaster_pipes[count][0]); // close READING END of pipe

        count++;
    }

    // ---------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------

    // NOW, LET'S FORK INTO THE CHILD PROCESSES

    while (count < workers) {

        pid_t tempPID = fork();
        if (tempPID == -1) exit(EXIT_FAILURE);
        else if (tempPID == 0) {
            arrayPID[count] = getpid(); // When PID == 0, it is child process (are we assuming the worker processes are running? Sent SIGSTOP)

        } // end of worker process running, would stop with SIGSTOP in worker
        count++;
    } // end of while loop

    // Wait for each child process to run
    int count2 = 0;
    int status;
    while (count2 < workers) {
        waitpid(arrayPID[count2], &status, 0);
    }
// ------------------------------------------------------------------------

// REPEATEDLY ASSIGNING PROBLEMS TO IDLE WORKERS AND POSTS RESULTS RECEIVED FROM WORKERS ( ithink at this point, all workers should be idle )

    // first, get the problem

    // waiting for children to exit

    sf_end();

    return EXIT_SUCCESS;
}

// From master to worker
void writeProblem(struct problem *selectedProblem, FILE *in) {

    // Includes both header and data
    char *charPtr = (char *) selectedProblem;
    if (charPtr == NULL) return exit(EXIT_FAILURE);
    int countPtr = 0;
    int overallSize = selectedProblem->size;
    while (countPtr != overallSize) {
        if (*charPtr == EOF) return exit(EXIT_FAILURE);
        fputc(*charPtr, in);
        charPtr++;
        countPtr++;
    }

}