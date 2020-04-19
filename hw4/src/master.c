#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include "debug.h"
#include "polya.h"

/*
 * master
 * (See polya.h for specification.)
 */
int master(int workers) {

    sf_start();

//                           INITIALIZE THE PROGRAM
// ------------------------------------------------------------------------
    pid_t arrayPID[workers]; // i dont think you have to malloc because only exist in this code block

    int masterToworker_pipes[workers][2]; // ARRAY (PIPE DESCRIPTORS), MASTER SENDS TO WORKER
    int workerTomaster_pipes[workers][2]; // ARRAY (PIPE DESCRIPTORS), WORKER SENDS TO MASTER

    int count = 0;
    pid_t tempPID;
    while (count < workers) {

        tempPID = fork();
        if (tempPID == -1) exit(EXIT_FAILURE);
        else if (tempPID == 0) {
            arrayPID[count] = getpid(); // When PID == 0, it is child process (are we assuming the worker processes are running? Sent SIGSTOP)

            if (pipe(masterToworker_pipes[count]) == -1) exit(EXIT_FAILURE);
            if (pipe(workerTomaster_pipes[count]) == -1) exit(EXIT_FAILURE);

            // Redirect standard input/output to pipes
            dup2(masterToworker_pipes[count][0], 0); // move pipe into stdin
            dup2(workerTomaster_pipes[count][1], 1); // move pipe into stdout
        }

        count++;
    }

    // redirect standard input/output to pipes
    // In unistd.h, 0 is file descriptor for standard input, 1 for standard output, 2 standard error output
    // pipe[0] = read end of the pipe, pipe[1] = write end of the pipe

    // close(pipefd1[1]); // closing the unused write end of pipe
    // close(pipefd2[0]); // closing the unused read end of pipe

    // Wait for each child process to run
    int count2 = 0;
    int status;
    while (count2 < workers) {
        waitpid(arrayPID[count2], &status, 0);
    }
// ------------------------------------------------------------------------

// REPEATEDLY ASSIGNING PROBLEMS TO IDLE WORKERS AND POSTS RESULTS RECEIVED FROM WORKERS

    // first, get the problem

    // waiting for children to exit



    sf_end();
    return EXIT_FAILURE;
}
