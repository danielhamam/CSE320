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

    // hold the two pipes for reading and writing
    int pipefd[2]; // undeclared array

    int count = 0;
    pid_t tempPID;
    while (count < workers) {
        if (pipe(pipefd) == -1) exit(EXIT_FAILURE);

        tempPID = fork();

        if (tempPID == -1) exit(EXIT_FAILURE);
        else if (tempPID == 0) arrayPID[count] = getpid(); // When PID == 0, it is child process (are we assuming the worker processes are running? Sent SIGSTOP)

        count++;
    }


    // Wait for each child process to run
    int count2 = 0;
    int status;
    while (count2 < workers) {
        waitpid(arrayPID[count2], &status, 0);
    }
// ------------------------------------------------------------------------

// REPEATEDLY ASSIGNING PROBLEMS TO IDLE WORKERS AND POSTS RESULTS RECEIVED FROM WORKERS

    // first, get the problem


    sf_end();
    return EXIT_FAILURE;
}
