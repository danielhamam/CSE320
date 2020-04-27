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
void SIGCHLD_handler();
void freeItems();

int workerReference; // to access as global variable

// Arrays
volatile sig_atomic_t *statesWorkers; // to hold workers' states
volatile sig_atomic_t *arrayPID;

int master(int workers) {

    sf_start();

    workerReference = workers;
    signal(SIGCHLD, SIGCHLD_handler); // signal handler for SIGCHLD

    // Block "any time the main program is actively involved in manipulating variables that are shared with a signal handler"
    sigset_t susMask;
    sigemptyset(&susMask);
    sigaddset(&susMask, SIGCHLD); // allowing all SIGNALS execpt SIGCHLD

// ---------------------------------------------------------------------------------------------------------------------------
//                           INITIALIZE THE PROGRAM
// ---------------------------------------------------------------------------------------------------------------------------

    // Redirect standard input/output to pipes ( pipe[0] = read end of the pipe, pipe[1] = write end of the pipe )
    // In unistd.h, 0 is file descriptor for standard input, 1 for standard output, 2 standard error output

    statesWorkers = malloc(workers * sizeof(sig_atomic_t));
    arrayPID = malloc(workers * sizeof(pid_t));

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

    sigprocmask(SIG_BLOCK, &susMask, NULL);

    int count2 = 0;
    // NOW, LET'S FORK INTO THE CHILD PROCESSES
    while (count2 < workers) {
        pid_t tempPID = fork();
        // if (tempPID == -1) exit(EXIT_FAILURE);
        if (tempPID == 0) {

            // change file descriptors in CHILD PROCESS (read = 0, write = 1)
            close(masterToworker_pipes[count2][1]); // child reads from M2W, close write end
            close(workerTomaster_pipes[count2][0]); // child writes to W2M, close read end

            // redirect to standard in/out
            dup2(masterToworker_pipes[count2][0], STDIN_FILENO); // child reads from M2W, make it stdin (WRITING END)
            dup2(workerTomaster_pipes[count2][1], STDOUT_FILENO); // child writes to W2M, make it stdout (READING END)

            // execute the worker program
            execl("bin/polya_worker", "", NULL);

        } // end of worker process running, would stop with SIGSTOP in worker
        else { // parent process

            arrayPID[count2] = tempPID; // When PID == 0, it is child process (are we assuming the worker processes are running? Sent SIGSTOP)
            // debug("Started to run parent process in fork");

            close(masterToworker_pipes[count2][0]); // parent writes to M2W, close read end
            close(workerTomaster_pipes[count2][1]); // parent reads from W2M, close write end

            // changing state of WORKERS
            statesWorkers[count2] = WORKER_STARTED;
            sf_change_state(arrayPID[count2], 0, WORKER_STARTED); // changing the state to started (0 is init state)
        }
        count2++;
    } // end of while loop

    sigprocmask(SIG_UNBLOCK, &susMask, NULL);

// ---------------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------

    // Let's wait for the workers to all be IDLE
    int counter2 = 0;

    while (counter2 < workers) {
        if (statesWorkers[counter2] != WORKER_IDLE) pause(); // pause for SIGCHLD to go to IDLE
        counter2++;
    }

    int lastMove = 0;
    struct problem *targetProblem;

// REPEATEDLY ASSIGNING PROBLEMS TO IDLE WORKERS AND POSTS RESULTS RECEIVED FROM WORKERS ( i think at this point, all workers should be idle )
    while (1) {

        // pause here and make sure every WORKER PROCESS is idle before proceeding

        pid_t targetWorker = -1; // the worker that we send a problem to, if found (-1 if not found)
        int worker_index = -1;

        // for loop to search for idle worker, find a problem and send to worker
        for (int c = 0; c < workers; c++) {
            if (statesWorkers[c] == WORKER_IDLE) {
                targetWorker = arrayPID[c]; // send problem to this worker (same as var)
                worker_index = c;
                // debug("Found IDLE Worker %d ", arrayPID[c]);
                break;
            }
        }

        if (targetWorker != -1) {

            // STEP --> LETS GET THE PROBLEM
            int nvars = workers; // NVAR = # of workers,
            int var = worker_index; // # VAR = ID of worker

            targetProblem = get_problem_variant(nvars, var); // we not have our problem
            if (targetProblem == NULL) {
                // debug("PROBLEM IS NULL");
                free(targetProblem);
                break; // it doesnt matter what state the workers are on, all problems are solved
            }

            // debug("Writing problem to Worker %d" , (int) targetWorker);
            FILE *fileInput = fdopen(masterToworker_pipes[worker_index][1], "w");
            writeProblem(targetProblem, fileInput);
            sf_send_problem(targetWorker, targetProblem);

            // SEND CONTINUE SIGNAL to WORKER
            kill(targetWorker, SIGCONT); // send worker the continue signal
            // --------------------------------------------
            statesWorkers[worker_index] = WORKER_CONTINUED;
            sf_change_state(arrayPID[worker_index], WORKER_IDLE, WORKER_CONTINUED);
            // ---------------------------------------------

            if (statesWorkers[worker_index] == WORKER_CONTINUED) {
                pause();  // To receive SIGCHLD to go to Running
            }
        }
        // Else, we just hop in this for loop

            // debug("FINISHING PART OF MAIN LOOPS");
            for (int finish_count = 0; finish_count < workers; finish_count++) {
                if (statesWorkers[finish_count] != WORKER_STOPPED) continue;
                else { // the result can now be read

                    // debug("Found Stopped Worker");

                    // Get Details of the Worker
                    pid_t foundWorker = arrayPID[finish_count];
                    int foundstoppedWorker_index = finish_count;

                    FILE *fileOutput = fdopen(workerTomaster_pipes[foundstoppedWorker_index][0], "r");
                    struct result *targetResult = readResult(fileOutput);
                    sf_recv_result(foundWorker, targetResult);
                    int result = post_result(targetResult, targetProblem); // will mark as "solved" if successful (aka no more variants of this type sent to problem)
                    // free(targetProblem);
                    // free(targetResult);

                    if (result == 0) { // aka if result is 0 (cancel all other workers running/solving)
                        // debug("Post Result = 0");
                        // free(targetProblem);
                        int checkingWorkers = 0;
                        while (checkingWorkers < workers) {
                            if (statesWorkers[checkingWorkers] == WORKER_CONTINUED || statesWorkers[checkingWorkers] == WORKER_RUNNING) {
                                kill(arrayPID[checkingWorkers], SIGHUP); // don't think you need to change state.
                                sf_cancel(arrayPID[checkingWorkers]); // cancel the workers that were sent SIGHUP
                                // pause();
                                // debug("CANCEL CANCEL CANCEL CANCEL");
                            }
                            checkingWorkers++;
                        } // end of WHILE loop
                    } // end of IF statement for result == 0

                    statesWorkers[finish_count] = WORKER_IDLE; // Set to IDLE
                    sf_change_state(arrayPID[finish_count], WORKER_STOPPED, WORKER_IDLE);

                    break; // go to next iteration of infinite loop

                } // end of IF STATEMENT for WORKER_STOPPED
            } // end of FOR LOOP

        if (lastMove) break;

    } // end of while loop

    // debug("Exited main loop");
    sigset_t mask1;
    sigfillset(&mask1);
    sigdelset(&mask1, SIGCHLD);

    sigprocmask(SIG_BLOCK, &mask1, NULL);

    // Here, let's send all the workers SIGTERM signal
    for (int i = 0; i < workers; i++) {
        // debug("Sending CONT & TERM Signals");
        pid_t target = arrayPID[i];
        kill(target, SIGCONT);
        kill(target, SIGTERM);
        while (statesWorkers[i] != WORKER_EXITED) pause();
    }

    // Let's free all the stuff
    freeItems();

    sigprocmask(SIG_UNBLOCK, &mask1, NULL);
    sf_end();
    return EXIT_SUCCESS;
}

void freeItems() {
    free((void *)statesWorkers);
    free((void *)arrayPID);
}

// From master to worker
void writeProblem(struct problem *selectedProblem, FILE *in) {

    // debug("Writing problem to Worker");

    // Includes both header and data
    char *charPtr = (char *) selectedProblem;
    // if (charPtr == NULL) return exit(EXIT_FAILURE);
    int countPtr = 0;
    int overallSize = selectedProblem->size;
    while (countPtr != overallSize) {
        // debug("WRITING A BYTE %d ", countPtr);
        // if (*charPtr == EOF) break;
        fputc(*charPtr, in);
        charPtr++;
        countPtr++;
    }
    fflush(in);
}

struct result *readResult(FILE *out) {

    // debug("Reading result from Worker");

    // First, read the SIZE variable (know how much to malloc)
    size_t count_size = 0;
    unsigned int tempSize = 0;
    while (count_size < sizeof(size_t)) {
        unsigned int byte = fgetc(out);
        // if (byte == EOF) break;;
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

    // Read padding?
    int countPadding = 0;
    while (countPadding < 5) {
        fgetc(out);
        countPadding++;
    }

    // Fourth, read the data section
    int sizeData = (int) (targetResult->size) - sizeof(struct result);
    // debug("SizeData : %d ", sizeData);
    // debug("TargetResult'size : %d ", (int) targetResult->size);
    // debug("Size of result: %d ", (int) sizeof(struct result));

    void *array2 = malloc(sizeData);
    if (array2 == NULL) exit(EXIT_FAILURE);
    memcpy(targetResult->data, array2, (size_t) sizeData); // Copy from the array we made to the read_problem->data)
    free(array2);

    // Char pointer points to beginning of data section
    int countData = 0;
    char *tempData = (char *) targetResult->data;
    while (countData < sizeData) {
        unsigned int byte = fgetc(out);
        // debug("BYTE: %d ", countData);
        // debug("BYTE: %d ",byte );
        *tempData = byte; // store char in this position
        countData++;
        tempData++;
    }
    // fflush(out);
    // debug("Result: size: %d, id: %d, failed: %d, data-size: %d", (int) targetResult->size, (int) targetResult->id, (int) targetResult->failed, (int) countData);
    return targetResult;

}

void SIGCHLD_handler(void) {

    // int wstatus;

    // 1st: STARTED --> IDLE
    // 2nd: IDLE --> CONTINUED
    // 3rd: CONTINUED --> RUNNING
    // 4th: RUNNING --> STOPPED
    // 5th: STOPPED --> IDLE
    // 6th: IDLE --> EXITED
    // 7th: CAN ABORT AT ANY MOMENT
    int infinite_loop = 1;
    int wstatus = 0;
    pid_t targetPID;
    int pidIndex = 0;

    while (infinite_loop) {
        // targetPID = waitpid(-1, &wstatus, WUNTRACED | WNOHANG | WCONTINUED);
        // debug("Master's SIGCHLD handler invoked");
        // First, find the index for later finding status of targetPID

        targetPID = waitpid(-1, &wstatus, WUNTRACED | WNOHANG | WCONTINUED);
        if (targetPID <= 0) break;

        for (int x = 0; x < workerReference; x++) {
            pid_t returnPID = arrayPID[x];
            if (returnPID == targetPID) { pidIndex = x; break; }
        }
        // now, find status of targetPID
        int pidStatus = statesWorkers[pidIndex];

        // -------------------------------------------------------------------------------------------------------
        // -------------------------------------------------------------------------------------------------------
        //                                           SIGNAL HANDLER CONDITIONS
        // -------------------------------------------------------------------------------------------------------
        // -------------------------------------------------------------------------------------------------------

        // I want the if statements to reflect the path, easier to read.
        int continuedToRunning = (WIFCONTINUED(wstatus) != 0);
        int runningToStopped = ( (WIFSTOPPED(wstatus) != 0) && pidStatus != WORKER_STARTED );
        int startedToIdle = ( (WIFSTOPPED(wstatus) != 0) && pidStatus == WORKER_STARTED );
        int idleToExited = (WIFEXITED(wstatus) != 0);

        // Let's make a switch statement to make it more readable (Case-By-Case)
        int outcome = -1;
        if (continuedToRunning) outcome = 1;
        else if (runningToStopped) outcome = 2;
        else if (startedToIdle) outcome = 3;
        else if (idleToExited) outcome = 4;
        // else outcome = -1;

        switch (outcome) {
            case -1: // PROCESS: SIGNAL ------> to ------> ABORTED
                statesWorkers[pidIndex] = WORKER_ABORTED;
                sf_change_state(targetPID, pidStatus, WORKER_ABORTED);
                break;
                // debug("Worker process %d has ABORTED", (int) targetPID);
            case 1: // PROCESS: CONTINUED ------> to ------> RUNNING
                statesWorkers[pidIndex] = WORKER_RUNNING;
                sf_change_state(targetPID, pidStatus, WORKER_RUNNING);
                // debug("Worker process %d switched from CONTINUED to RUNNING", (int) targetPID);
                break;
            case 2: // PROCESS: RUNNING ------> to ------> STOPPED
                statesWorkers[pidIndex] = WORKER_STOPPED;
                sf_change_state(targetPID, pidStatus, WORKER_STOPPED);
                // debug("Worker process %d switched from RUNNING to STOPPED", (int) targetPID);
                break;
            case 3: // PROCESS: STARTED ------> to ------> IDLE
                statesWorkers[pidIndex] = WORKER_IDLE;
                sf_change_state(targetPID, pidStatus, WORKER_IDLE);
                // debug("Worker process %d switched from STARTED to IDLE", (int) targetPID);
                break;
            case 4: // PROCESS: IDLE ------> to ------> EXITED
                statesWorkers[pidIndex] = WORKER_EXITED;
                sf_change_state(targetPID, pidStatus, WORKER_EXITED);
                // debug("Worker process %d switched from IDLE to EXITED", (int) targetPID);
                break;
        } // end of switch statement
    } // end of while loop
} // end of SIGCHLD handler