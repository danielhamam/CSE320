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

volatile sig_atomic_t *statesWorkers = 0; // to hold workers' states

int master(int workers) {

    sf_start();

// ---------------------------------------------------------------------------------------------------------------------------
//                           INITIALIZE THE PROGRAM
// ---------------------------------------------------------------------------------------------------------------------------

    // Redirect standard input/output to pipes ( pipe[0] = read end of the pipe, pipe[1] = write end of the pipe )
    // In unistd.h, 0 is file descriptor for standard input, 1 for standard output, 2 standard error output

    statesWorkers = malloc(workers * sizeof(sig_atomic_t));
    pid_t arrayPID[workers]; // i dont think you have to malloc because only exist in this code block

    int masterToworker_pipes[workers][2]; // ARRAY (PIPE DESCRIPTORS), MASTER SENDS TO WORKER
    int workerTomaster_pipes[workers][2]; // ARRAY (PIPE DESCRIPTORS), WORKER SENDS TO MASTER

    // Create pipes for every single worker
    int count = 0;
    while (count < workers) {
        if (pipe(masterToworker_pipes[count]) == -1) exit(EXIT_FAILURE);
        if (pipe(workerTomaster_pipes[count]) == -1) exit(EXIT_FAILURE);
        count++;
    }

// ---------------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------

    // NOW, LET'S FORK INTO THE CHILD PROCESSES

    while (count < workers) {

        pid_t tempPID = fork();
        arrayPID[count] = tempPID; // When PID == 0, it is child process (are we assuming the worker processes are running? Sent SIGSTOP)
        if (tempPID == -1) exit(EXIT_FAILURE);
        else if (tempPID == 0) {

            debug("Started to work with Worker %d ", getpid());

            sf_change_state(getpid(), 0, WORKER_STARTED); // changing the state to started (0 is init state)

            // change file descriptors in CHILD PROCESS (read = 0, write = 1)
            close(masterToworker_pipes[count][1]); // child reads from M2W, close write end
            close(workerTomaster_pipes[count][0]); // child writes to W2M, close read end

            // redirect to standard in/out
            dup2(masterToworker_pipes[count][1],0); // child reads from M2W, make it stdin (WRITING END)
            dup2(workerTomaster_pipes[count][0],1); // child writes to W2M, make it stdout (READING END)

            // execute the worker program
            execl("bin/polya_worker", "bin/polya_worker", NULL);

        } // end of worker process running, would stop with SIGSTOP in worker
        else { // parent process
            close(masterToworker_pipes[count][0]); // parent writes to M2W, close read end
            close(workerTomaster_pipes[count][1]); // parent reads from W2M, close write end
        }
        count++;
    } // end of while loop

// ---------------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------

// REPEATEDLY ASSIGNING PROBLEMS TO IDLE WORKERS AND POSTS RESULTS RECEIVED FROM WORKERS ( i think at this point, all workers should be idle )
    while (1) {

        // for loop to search for idle worker, find a problem and send to worker
        for (int c = 0; c < workers; c++) {
            pid_t pid = arrayPID[c];

        }

    }

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