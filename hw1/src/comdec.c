#include "const.h"
#include "sequitur.h"
#include "debug.h"
#include "stdio.h"

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
    // in and out are file pointers
    // FILE *opened_input = fopen(in, "r");

    // if (opened_input == NULL) {
    //     return EOF;
    // }

    // while(1) {

    // // read single character at a time
    // // char file_char = fgetc(opened_input);

    // // check if terminated... end
    // if (feof(opened_input)) {
    //     break; // break out of infinite while loop
    // }

    // }



    return EOF; // end of file
}

// helper function to read UTF-8 encoded character from input
// identify special marker bytes (to parse compressed data transmission into blocks and rules)
int check_read_amount(unsigned int ch) {
    // From piazza: "0=1 byte, 110=2 bytes, 1110=3 bytes, 11110=4 bytes"
    // ch is binary 8 bit value

    unsigned int check_first_bit = (ch >> 7);
    unsigned int check_three_bits = (ch >> 5); // 7 in binary is 111.
    unsigned int check_four_bits = (ch >> 4); // 15 in binary is 1111.
    unsigned int check_five_bits = (ch >> 3); // 31 in binarty is 11111.

    if (check_first_bit == 0b0) {
        // printf("HEY1");
        return 1; // read 1 byte
    }
    else if (check_three_bits == 0b110) { // 110 in binary is 6 in decimal.
        // printf("HEY2");
        return 2; // read 2 bytes
    }
    else if (check_four_bits == 0b1110) { // 1110 in binary is 14 in decimal
        // printf("HEY3");
        return 3; // read 3 bytes
    }
    else if (check_five_bits == 0b11110) { // 11110 in binary is 30 in decimal
        // printf("HEY4");
        return 4; // read 4 bytes
    }
    else {
        return -1;
    }
}

/**
* Recursive function to output decompressed data bytes
*/
void decompress_helper(SYMBOL *rule, FILE *out) {

    if (rule->next == rule) return;

        printf("RULE VALUE: %d\n", (rule->value));

        if ((rule->value) < FIRST_NONTERMINAL) { // terminal value
           fputc(rule->value, out);
           decompress_helper(rule->next, out);
        }
        else if ((rule->value) >= FIRST_NONTERMINAL) {
            // printf("HEY2\n");
            // printf("RULE PLACED AT %d\n", rule->value);
            SYMBOL *rule1 = *(rule_map + (rule->value));
            decompress_helper( rule1->next , out); // recursive call
        }
}

int readCharacter(unsigned int result, FILE *in) {

        int read_amount = check_read_amount(result); // gives us how many bytes we have to read for character
        int new_value = 0;
        int results_together = 0;
        if (read_amount == 1) {

            result = (result & 0b01111111); // get rid of first zero;
            new_value = result;
            // now you have the binary code of the character.
        }
        else if (read_amount == 2) {

            result = (result & 0b00011111);  // remove 110
            unsigned int result2 = fgetc(in);
            result2 = (result2 & 0b00111111); // remove the first two bytes 10
            // combine into one number
            results_together = results_together | result; // result is 5 bits
            results_together = results_together << 6;
            results_together = results_together | result2; // result2 is 6 bits.
            new_value = results_together;
        }
        else if (read_amount == 3) {

            result = (result & 0b00001111); // remove 1110
            unsigned int result2 = fgetc(in); // next byte #2
            result2 = (result2 & 0b00111111);
            unsigned int result3 = fgetc(in); // next byte #3
            result3 = (result3 & 0b00111111);
            // combine into one binary number
            results_together = results_together | result;
            results_together = results_together << 6;
            results_together = results_together | result2;
            results_together = results_together << 6;
            results_together = results_together | result3;
            new_value = results_together;
        }
        else if (read_amount == 4) {

            result = (result & 0b00000111); // remove 11110
            unsigned int result2 = fgetc(in); // next byte #2
            result2 = (result2 & 0b00111111);
            unsigned int result3 = fgetc(in); // next byte #3
            result3 = (result3 & 0b00111111);
            unsigned int result4 = fgetc(in); // next byte #4
            result4 = (result4 & 0b00111111);
            // combine into one binary number
            results_together = results_together | result;
            results_together = results_together << 6;
            results_together = results_together | result2;
            results_together = results_together << 6;
            results_together = results_together | result3;
            results_together = results_together << 6;
            results_together = results_together | result4;
            new_value = results_together;
        }
        return new_value;
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

    unsigned int result = 0;
    int firsttime = 0; // to check if head rule has a symbol with it or not
    SYMBOL *head_rule; // CURRENTLY SELECTED RULE
    unsigned int firstrulefound = 0;

    while (1) {

    // printf("LOOP");
        result = fgetc(in); // unsigned char from fgetc

        // Check for start of transmission:
        if (result == 0x81) {
            continue;
        }

        // Check for end of transmission:
        else if (result == 0x82) { // check if it is END OF FILE MARKER.
            break;
        }

        // Check for start of block:
        else if (result == 0x83) {
            firstrulefound = 0; // reset head_rule
            firsttime = 0;
            init_rules();
            init_symbols();
            continue; // go to next iteration
        }

        // Check for end of block:
        else if (result == 0x84) {
            // From Piazza: "you want to reset everything" between each block
            printf("END BLOCK\n");
            printf("------------------------------------------\n");
            // printf("THIS IS MAIN_NEXT: %x\n", (main_rule->next)->value);

         printf("\nRULES LINKED LIST:\n");
         SYMBOL *headptr = main_rule;
         while(headptr->nextr != main_rule) {
            printf(" #: %x ", headptr->value);
            headptr = headptr->nextr;
         }
         printf("\nSYMBOLS LINKED LIST IN MAIN RULE:\n");
         SYMBOL *headptr2 = main_rule;
         while(headptr2->next != main_rule) {
            printf(" #: %x ", headptr2->value);
            headptr2 = headptr2->next;
         }
        printf("\n------------------------------------------\n");
            // return 0;
            decompress_helper( main_rule, out );
            continue; // go to next iteration
        }

        // Rule delimiter
        else if (result == 0x85) {
            // Reset so new head rule
            // return 0;
            printf(" END RULE\n");
            firsttime = 0;
            firstrulefound = 0;
            head_rule = NULL;
            continue;
        }

        unsigned int new_value = readCharacter(result, in);
        // ------------------------------------------------------
        // NOW THEY ALL GO TO THIS:
        if (new_value > 255) {
            printf("%x ", new_value);
            printf("==>");
        }
         else if (new_value == 10) printf("\n ");
         else if (new_value == 32) printf("[ws] ");
         else printf("%c ", new_value);
        // ------------------------------------------------------

        // Piazza: Rule parameters all supposed to be null while reading. Assign after reading.
        if (new_value < FIRST_NONTERMINAL) {
            // make new symbol
            SYMBOL *newsymb = new_symbol(new_value, NULL);
            // Connect
            if (firsttime == 0) {
                // ONLY HEAD, attach first symbol.
                head_rule->next = newsymb;
                newsymb->prev = head_rule;
                head_rule->prev = newsymb;
                newsymb->next = head_rule;
                firsttime = 1;
            }
            else {
                // Second, third, fourth... symbol is attached...
                head_rule->prev->next = newsymb; // THE ERROR
                newsymb->prev = head_rule->prev;
                newsymb->next = head_rule;
                head_rule->prev = newsymb;
            }
            // return 0;
        }
        else if ((new_value >= FIRST_NONTERMINAL) && firstrulefound == 0) { // boolean 0 means first rule not found
            // make new HEAD rule
            head_rule = new_rule(new_value);
            add_rule(head_rule);
            *(rule_map + head_rule->value) = head_rule; // increments by value and sets it to rule
            firsttime = 0; // if 0, there's only head rule. if 1, theres symbols connected
            firstrulefound = 1;
            // printf("MAIN RULE");
            // RULE parameter can be used to specify a rule having that nonterminal at its head.
        }
        else if ((new_value >= FIRST_NONTERMINAL) && firstrulefound != 0) { // boolean 0 means we already found first rule
            // there is already a head rule
            SYMBOL *new_rule2 = new_symbol(new_value, NULL);
            // return 0;
            // Connect

            if (firsttime == 0) {
                // means theres only the head rule made.
                // printf("DOING THIS");
                head_rule->next = new_rule2;
                new_rule2->prev = head_rule;
                head_rule->prev = new_rule2;
                new_rule2->next = head_rule;
                firsttime = 1;
            }
            else {
                // Third, fourth, fifth symbol...
                // printf("DOING THIS 2");
                // printf("ADDRES: %p ", head_rule->prev->next);
                // return 0;
                head_rule->prev->next = new_rule2; // THIS IS THE ERROR.
                new_rule2->prev = head_rule->prev;
                head_rule->prev = new_rule2;
                new_rule2->next = head_rule;
            }
        }
    } // end of while(1) loop

    printf("OUT OF WHILE LOOP");
    fflush(out); // ensure no output remains buffered in memory.
    return 0; // file does not follow format
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

    char **replace_argv = argv;
    if (argc > 1) { // if its more than just bin/sequitur (which is first argument)

        replace_argv++; // now we have focus on position argv[1], where argument(s) should start.
        char* argpointer1 = *replace_argv; // points to memory of '-' argv[1][0]

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
                // printf("position: %c", position);
                if (position != 0) {
                    // printf("failed: there's a char after h");
                    return -1;
                }
                // printf("h --> success");
                global_options = 1; // first LEAST significant bit is 1
                // printf("global options = %d", global_options);
                return 0; // true because rest of arguments ignored. (EXIT_SUCCESS)
            }
            else if (*argpointer1 == 'c') {

                // Check if there's a char after -c_
                argpointer1++;
                char* argpointer_future = argpointer1;
                char position = *argpointer_future;
                // printf("position: %c", position);
                if (position != 0) {
                    // printf("failed: there's a char after c");
                    return -1;
                }

                if (argc == 2) {
                    // printf("c --> success");
                    global_options = 0x0400;// default 1024kb block size (0x0400)
                    global_options = global_options << 16;
                    global_options = global_options | 2; // second LEAST significant bit is 1
                    // if (global_options == 0x4000002 ) {
                    //     printf("global options = %d", global_options);
                    // }
                    return 0; // true because -c is first and only argument. (no optional)
                }
                replace_argv++; // now we have focus on position argv[2] (after 1st argument.)
                char* argpointer2 = *replace_argv; // points to memory of next argument argv[2][0]
                char arg_position2 = *argpointer2; // explicit value of position argv[2]
                if (arg_position2 == '-') {
                    // second arg is an argument
                    argpointer2++;
                    if (*argpointer2 == 'b' && argc == 4) {

                        // Check if there's a char after -b_
                        argpointer2++;
                        char* argpointer_future2 = argpointer2;
                        char position2 = *argpointer_future2;
                        // printf("position: %c", position2);
                        if (position2 != 0) {
                            // printf("failed: there's a char after b");
                            return -1;
                        }
                        // account for nothing after -b (default)
                        if (argc == 3) {
                            // printf("Nothing after B error");
                            return -1; //nothing after b
                        }
                        // printf("checking for b");
                        // now check that after b is digit checked.
                        replace_argv++; //now focused on 3rd argument after -b
                        char* argpointer3 = *replace_argv;
                        char* holderofposition = *replace_argv;
                        char arg_position3 = *argpointer3; // explicit value of position argv[3]
                        int size = 0;
                        // CHECK IF DIGITS ARE VALID:
                        while (arg_position3 != '\0') {

                            // verify it is a digit
                            if (arg_position3 == '0' || arg_position3 == '1' || arg_position3 == '2' || arg_position3 == '3' || arg_position3 == '4' || arg_position3 == '5' ||
                                arg_position3 == '6' || arg_position3 == '7' || arg_position3 == '8' || arg_position3 == '9') {
                                // next char (digit) in this argument
                                // printf("%c", arg_position3);
                                argpointer3++;
                                arg_position3 = *argpointer3;
                                size++;
                            }
                            else {
                                // printf("b--> failed");
                                return -1; // false because digit contains letter (EXIT_FAILURE)
                            }
                        }

                        // Check if B is less than 1

                        if (size == 1) {
                            char offset_value = *holderofposition;
                            if (offset_value < '1') { // comparison using ASCII
                                // printf("Failure --> b is less than 1");
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
                                        // printf("Success --> b is within range");
                                        return 0;
                                    }
                                }
                                if (size == 3) { // we're assuming first digit is 1
                                    if (offset_value > '0') {
                                        // printf("Failure --> b is larger than 1024");
                                        return -1; // failure
                                    }
                                }
                                if (size == 2) { // we're assuming first digit is 1 and second is 0
                                    if (offset_value > '2') {
                                        // printf("Failure --> b is larger than 1024");
                                        return -1;
                                    }
                                }
                                if (size == 1) {
                                    if (offset_value > '4') {
                                        // printf("Failure --> b is larger than 1024");
                                        return -1;
                                    }
                                }
                                holderofposition++; // next character (integer)
                                size--;
                            }
                        }
                        else if (size > 4) {
                            // printf("Failure --> b is larger than 1024");
                            return -1;
                        }
                        // Passed checks:
                        // printf("Passed all checks for b");
                        global_options = atoiPositive(*replace_argv);
                        global_options =  global_options << 16;
                        global_options = global_options | 2; // c bit is 1 (so we or it with blocksize
                        // printf("GLOBAL OPTIONS IS EQUAL TO: %d", global_options);
                        return 0;
                        } // end of if statement for b
                        // printf("Not correct format -> failure");
                        return -1; // b is valid, so return success
                    }
                }
            else if (*argpointer1 == 'd') {

                // Check if there's a char after -b_
                argpointer1++;
                char* argpointer_future2 = argpointer1;
                char position = *argpointer_future2;
                // printf("position: %c", position);
                if (position != 0) {
                    // printf("failed: there's a char after d");
                    return -1;
                }
                if (argc == 2) {
                    // printf("d --> success");
                    global_options = 4; // third LEAST significant bit.
                    // printf("global_options = %d", global_options);
                    return 0;
                }
                else {
                    // printf("d --> failed");
                    return -1; // there are arguments after d.
                }
            }
    } // end of '-' if statement
    }
    // printf("Incorrect format: failure");
    return -1;
}