#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#include "debug.h"
#include "polya.h"

/*
 * master
 * (See polya.h for specification.)
 */

struct problem *generateProblem();
void writeProblem(struct problem *problem, FILE *in);
struct result *readResult(FILE *out);

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

        pid_t targetWorker = -1; // the worker that we send a problem to, if found (-1 if not found)
        int worker_index;

        // for loop to search for idle worker, find a problem and send to worker
        for (int c = 0; c < workers; c++) {
            if (statesWorkers[c] == WORKER_IDLE) {
                targetWorker = arrayPID[c]; // send problem to this worker (same as var)
                worker_index = c;
            }
        }

        // NVAR = # of workers, VAR = ID of worker
        int nvars = workers;
        int var = targetWorker;

        // Get the problem, and write to worker
        struct problem *targetProblem = get_problem_variant(nvars, var); // we not have our problem
        FILE *fileInput = fdopen(masterToworker_pipes[worker_index][1], "w");
        kill(targetWorker, SIGCONT); // send worker the continue signal
        writeProblem(targetProblem, fileInput);

        // When SIGCHLD received


    }

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

struct result *readResult(FILE *out) {

    // First, read the SIZE variable (know how much to malloc)
    size_t count_size = 0;
    unsigned int tempSize = 0;
    while (count_size < sizeof(size_t)) {
        unsigned int byte = fgetc(out);
        if (byte == EOF) exit(EXIT_FAILURE);
        tempSize += byte;
        count_size++;
    }

    // --------------------------------------------------------------------------------------------------------
    // --------------------------------------------------------------------------------------------------------

    // Now we can malloc to the right size

    struct result *targetResult = (struct result *) malloc(tempSize);
    targetResult->size = (size_t) tempSize;

    // Second, read the short ID variable
    int count_ID = 0;
    unsigned int tempID = 0;
    while (count_ID < sizeof(short)) {
        unsigned int byte = fgetc(out);
        if (byte == EOF) exit(EXIT_FAILURE);
        tempID += byte;
        count_ID++;
    }
    targetResult->id = (short) tempID;

    // Third, read the char failed variable
    unsigned int tempFailed = fgetc(out);
    targetResult->failed = tempFailed;

    // Fourth, read the data section
    int sizeData = (int) (targetResult->size) - sizeof(struct result);

    void *array2 = malloc(sizeData);
    if (array2 == NULL) EXIT_FAILURE;
    memcpy(targetResult->data, array2, (size_t) sizeData); // Copy from the array we made to the read_problem->data)
    free(array2);

    // Char pointer points to beginning of data section
    int countData = 0;
    char *tempData = targetResult->data;
    while (countData < sizeData) {
        unsigned int byte = fgetc(out);
        // debug("BYTE: %d ",byte );
        if (byte == EOF) exit(EXIT_FAILURE);
        *tempData = byte; // store char in this position
        countData++;
        tempData++;
    }

    return targetResult;

    }
