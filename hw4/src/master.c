#include <stdlib.h>
#include <unistd.h>

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
    pid_t arrayPID[workers];
    // hold the two pipes for reading and writing
    int pipefd[2]; // undeclared array

    int count = 0;
    pid_t tempPID;
    while (count < workers) {
        if (pipe(pipefd) == -1) exit(EXIT_FAILURE);

        tempPID = fork();

        if (tempPID == -1) exit(EXIT_FAILURE);
        else arrayPID[count] = tempPID;

        count++;
    }
// ------------------------------------------------------------------------




    sf_end();
    return EXIT_FAILURE;
}
