#include "const.h"
#include "sequitur.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

/*
 * You may modify this file and/or move the functions contained here
 * to other source files (except for main.c) as you wish.
 *
 * IMPORTANT: You MAY NOT use any array brackets (i.e. [ and ]) and
 * you MAY NOT declare any arrays or allocate any storage with malloc().
 * The purpose of this restriction is to force you to use pointers.
 * Variables to hold the pathname of the current file or directory
 * as well as other data have been pre-declared for you in const.h.
 * You must use those variables, rather than declaring your own.
 * IF YOU VIOLATE THIS RESTRICTION, YOU WILL GET A ZERO!
 *
 * IMPORTANT: You MAY NOT use floating point arithmetic or declare
 * any "float" or "double" variables.  IF YOU VIOLATE THIS RESTRICTION,
 * YOU WILL GET A ZERO!
 */

// Function to convert character to char[] to int
int atoiPositive(char* string) {
    int temp = 0;
    if (*string != 0) {
        while (*string != '\0') {
            char c = *string;
            if (c >= '0' && c <= '9') {
                int val = c - 48; // SUBTRACT ASCII VALUES.
                temp = temp * 10;
                temp = temp + val;
            } else {
                return -1;
            }
            string++;
            }
        }
        return temp;
    }

/**
 * Main compression function.
 * Reads a sequence of bytes from a specified input stream, segments the
 * input data into blocks of a specified maximum number of bytes,
 * uses the Sequitur algorithm to compress each block of input to a list
 * of rules, and outputs the resulting compressed data transmission to a
 * specified output stream in the format detailed in the header files and
 * assignment handout.  The output stream is flushed once the transmission
 * is complete.
 *
 * The maximum number of bytes of uncompressed data represented by each
 * block of the compressed transmission is limited to the specified value
 * "bsize".  Each compressed block except for the last one represents exactly
 * "bsize" bytes of uncompressed data and the last compressed block represents
 * at most "bsize" bytes.
 *
 * @param in  The stream from which input is to be read.
 * @param out  The stream to which the block is to be written.
 * @param bsize  The maximum number of bytes read per block.
 * @return  The number of bytes written, in case of success,
 * otherwise EOF.
 */
int compress(FILE *in, FILE *out, int bsize) {
    // To be implemented.
    return EOF;
}

/**
 * Main decompression function.
 * Reads a compressed data transmission from an input stream, expands it,
 * and and writes the resulting decompressed data to an output stream.
 * The output stream is flushed once writing is complete.
 *
 * @param in  The stream from which the compressed block is to be read.
 * @param out  The stream to which the uncompressed data is to be written.
 * @return  The number of bytes written, in case of success, otherwise EOF.
 */
int decompress(FILE *in, FILE *out) {
    // To be implemented.
    return EOF;
}

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the selected program options will be set in the
 * global variable "global_options", where they will be accessible
 * elsewhere in the program.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * Refer to the homework document for the effects of this function on
 * global variables.
 * @modifies global variable "global_options" to contain a bitmap representing
 * the selected options.
 */
int validargs(int argc, char **argv) {

    printf("count: %d",  argc);
    if (argc > 1) { // if its more than just bin/sequitur (which is first argument)

        argv++; // now we have focus on position argv[1], where argument(s) should start.
        char* argpointer1 = *argv; // points to memory of '-' argv[1][0]

        char arg_position1 = *argpointer1; // explicit value of position argv[1]

        // CHECK ARGV[1]
        // **argv = double dereference
        if (arg_position1 == '-') { // check if there is an argument.

            // check next letter after '-'
            argpointer1++; // points to memory of argv[1][1]
            if (*argpointer1 == 'h') {

                // Check if there's a char after -h
                argpointer1++;
                char* argpointer_future = argpointer1;
                char position = *argpointer_future;
                printf("position: %c", position);
                if (position != 0) {
                    printf("failed: there's a char after h");
                    return -1;
                }

                printf("h --> success");
                global_options = 1; // first LEAST significant bit is 1
                printf("global options = %d", global_options);
                return 0; // true because rest of arguments ignored. (EXIT_SUCCESS)
            }
            else if (*argpointer1 == 'c') {

                // Check if there's a char after -c_
                argpointer1++;
                char* argpointer_future = argpointer1;
                char position = *argpointer_future;
                printf("position: %c", position);
                if (position != 0) {
                    printf("failed: there's a char after c");
                    return -1;
                }

                if (argc == 2) {
                    printf("c --> success");
                    global_options = 0x0400;// default 1024kb block size (0x0400)
                    global_options = global_options << 16;
                    global_options = global_options | 2; // second LEAST significant bit is 1
                    if (global_options == 0x4000002 ) {
                        printf("global options = %d", global_options);
                    }
                    return 0; // true because -c is first and only argument. (no optional)
                }
                argv++; // now we have focus on position argv[2] (after 1st argument.)
                char* argpointer2 = *argv; // points to memory of next argument argv[2][0]
                char arg_position2 = *argpointer2; // explicit value of position argv[2]
                if (arg_position2 == '-') {
                    // second arg is an argument
                    argpointer2++;
                    if (*argpointer2 == 'b' && argc == 4) {

                        // Check if there's a char after -b_
                        argpointer2++;
                        char* argpointer_future2 = argpointer2;
                        char position2 = *argpointer_future2;
                        printf("position: %c", position2);
                        if (position2 != 0) {
                            printf("failed: there's a char after b");
                            return -1;
                        }
                        // account for nothing after -b (default)
                        if (argc == 3) {
                            printf("Nothing after B error");
                            return -1; //nothing after b
                        }

                        printf("checking for b");
                        // now check that after b is digit checked.
                        argv++; //now focused on 3rd argument after -b
                        char* argpointer3 = *argv;
                        char* holderofposition = *argv;
                        char arg_position3 = *argpointer3; // explicit value of position argv[3]
                        int size = 0;
                        // CHECK IF DIGITS ARE VALID:
                        while (arg_position3 != '\0') {

                            // verify it is a digit
                            if (arg_position3 == '0' || arg_position3 == '1' || arg_position3 == '2' || arg_position3 == '3' || arg_position3 == '4' || arg_position3 == '5' ||
                                arg_position3 == '6' || arg_position3 == '7' || arg_position3 == '8' || arg_position3 == '9') {
                                // next char (digit) in this argument
                                printf("%c", arg_position3);
                                argpointer3++;
                                arg_position3 = *argpointer3;
                                size++;
                            }
                            else {
                                printf("b--> failed");
                                return -1; // false because digit contains letter (EXIT_FAILURE)
                            }
                        }

                        // Check if B is less than 1

                        if (size == 1) {
                            char offset_value = *holderofposition;
                            if (offset_value < '1') { // comparison using ASCII
                                printf("Failure --> b is less than 1");
                                return -1; // EXIT_FAILURE
                            }
                        }
                        // Check if B is greater than 1024
                        else if (size == 4) {
                            char offset_value;
                            // check eachk one position to be lower than 1024
                            while (size != 0) {
                                offset_value = *holderofposition;


                                if (size == 4) { // first digit in 4 sequence
                                    if (offset_value != '1') {
                                        // means first digit is zero, something like '0024'
                                        printf("Success --> b is within range");
                                        return 0;
                                    }
                                }
                                if (size == 3) { // we're assuming first digit is 1
                                    if (offset_value > '0') {
                                        printf("Failure --> b is larger than 1024");
                                        return -1; // failure
                                    }
                                }
                                if (size == 2) { // we're assuming first digit is 1 and second is 0
                                    if (offset_value > '2') {
                                        printf("Failure --> b is larger than 1024");
                                        return -1;
                                    }
                                }
                                if (size == 1) {
                                    if (offset_value > '4') {
                                        printf("Failure --> b is larger than 1024");
                                        return -1;
                                    }
                                }
                                holderofposition++; // next character (integer)
                                size--;
                            }
                        }
                        else if (size > 4) {
                            printf("Failure --> b is larger than 1024");
                            return -1;
                        }
                        // Passed checks:
                        printf("Passed all checks for b");
                        global_options = atoiPositive(*argv);
                        global_options =  global_options << 16;
                        global_options = global_options | 2; // c bit is 1 (so we or it with blocksize
                        printf("GLOBAL OPTIONS IS EQUAL TO: %d", global_options);
                        return 0;
                        } // end of if statement for b

                        printf("Not correct format -> failure");
                        return -1; // b is valid, so return success
                    }
                }
            else if (*argpointer1 == 'd') {

                // Check if there's a char after -b_
                argpointer1++;
                char* argpointer_future2 = argpointer1;
                char position = *argpointer_future2;
                printf("position: %c", position);
                if (position != 0) {
                    printf("failed: there's a char after d");
                    return -1;
                }
                if (argc == 2) {
                    printf("d --> success");
                    global_options = 4; // third LEAST significant bit.
                    printf("global_options = %d", global_options);
                    return 0;
                }
                else {
                    printf("d --> failed");
                    return -1; // there are arguments after d.
                }
            }
    } // end of '-' if statement
    }
    printf("Incorrect format: failure");
    return -1;
}