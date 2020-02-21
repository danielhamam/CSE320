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

// To convert each symbol in each rule to UTF
int convert_to_utf(SYMBOL *rule , FILE *out) {

    SYMBOL *currentRule = rule;
    unsigned int value = 0;

    while (currentRule->nextr != rule) {

    // go through each symbol
        SYMBOL *currentptr = currentRule;
        currentptr = currentptr->next;

        while (currentptr->next != currentRule) {

            currentptr = currentptr->next;
            value = currentptr->value;

            if (value >= 0 && value <= 127) { // 1 byte (0)
                int new_val = value & 0x0111111;
                fputc(new_val, out);
            }
            else if (value >= 128 && value <= 2047) { // 2 bytes (110 10)
                int first_half = (value >> 6) | (0b11000000); // 110
                int second_half = (value & 0b00111111) | 0b10000000;

                fputc(first_half, out);
                fputc(second_half, out);
            }
            else if (value >= 2048 && value <= 65535) { // 3 bytes (1110 10 10)
                int first_half = (value >> 12) | 0b11100000; // 11110
                int second_half = (value >> 6) | 0b10000000; // 10
                int third_half = value | 0b10000000; // 10

                fputc(first_half, out);
                fputc(second_half, out);
                fputc(third_half, out);
            }
            else if (value >= 65536 && value <= 1048575) { // 4 bytes (11110 10 10 10)
                int first_half = (value >> 18) | 0b11110000; // 111110
                int second_half = (value >> 12) | 0b10000000; // 10
                int third_half = (value >> 6) | 0b10000000; // 10
                int fourth_half = (value | 0b10000000); // 10

                fputc(first_half, out);
                fputc(second_half, out);
                fputc(third_half, out);
                fputc(fourth_half, out);
            }
        } // end of while loop (1)

        currentRule = currentRule->nextr;
        fputc(0x85, out);
    } // end of while loop (2)
    return 0;
} // end of function

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

    unsigned int result = 0;
    int amount_bytes = 0;
    int readBytes = 0;
    bsize = bsize * 1024;

    fputc(0x81, out); // SOT MARKER

    while (1) { // "MAIN LOOP"

        readBytes = 0;

        // "Reset Everything"
        init_symbols();
        init_rules();
        init_digram_hash();

        // "Set Main_Rule"
        SYMBOL *newrule = new_rule(next_nonterminal_value);
        add_rule(newrule);
        next_nonterminal_value++;

        fputc(0x83, out); // Start of Block Marker

        // 0x82 = EOT, 0x85 = RULE DELIMITER, 0x83 = SOB, 0x81 = SOT
        while(readBytes < bsize) {

            result = fgetc(in); // get next byte

            if (result == EOF) {
                break;
            }

            SYMBOL *newsym = new_symbol(result, NULL); // NULL because non-terminal
            insert_after(main_rule->prev, newsym); // Error here
            check_digram(main_rule->prev->prev);
            readBytes++;

        } // end of while loop (1)_

        fputc(0x82, out); // End of Block Marker
        convert_to_utf(main_rule, out);

        if (result == EOF) {
            break;
        }

    } // end of while loop (2)

    fputc(0x84, out); // EOT MARKER

    fflush(out);
    return amount_bytes; // end of file
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
int bytes_read = 0;
int decompress_helper(SYMBOL *rule, FILE *out) {

    SYMBOL *currentptr = rule;

    while (currentptr->next != rule) {

        currentptr = currentptr->next;

        if ((currentptr->value) < FIRST_NONTERMINAL) { // terminal value
           fputc(currentptr->value, out);
           bytes_read++;
        }
        else if ((currentptr->value) >= FIRST_NONTERMINAL) {
            SYMBOL *rule1 = *(rule_map + (currentptr->value));
            decompress_helper( rule1 , out); // recursive call
        }
    }
    return bytes_read;
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
    int amount_bytes = 0;

    while (1) {

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
            amount_bytes = decompress_helper(main_rule, out);
            continue; // go to next iteration
        }

        // Rule delimiter
        else if (result == 0x85) {
            // Reset so new head rule
            firsttime = 0;
            firstrulefound = 0;
            head_rule = NULL;
            continue;
        }

        unsigned int new_value = readCharacter(result, in);

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
        }
        else if ((new_value >= FIRST_NONTERMINAL) && firstrulefound == 0) { // boolean 0 means first rule not found
            // make new HEAD rule
            head_rule = new_rule(new_value);
            add_rule(head_rule);
            *(rule_map + head_rule->value) = head_rule; // increments by value and sets it to rule
            firsttime = 0; // if 0, there's only head rule. if 1, theres symbols connected
            firstrulefound = 1;
            // RULE parameter can be used to specify a rule having that nonterminal at its head.
        }
        else if ((new_value >= FIRST_NONTERMINAL) && firstrulefound != 0) { // boolean 0 means we already found first rule
            // there is already a head rule
            SYMBOL *new_rule2 = new_symbol(new_value, NULL);

            if (firsttime == 0) {
                // means theres only the head rule made.
                head_rule->next = new_rule2;
                new_rule2->prev = head_rule;
                head_rule->prev = new_rule2;
                new_rule2->next = head_rule;
                firsttime = 1;
            }
            else {
                // Third, fourth, fifth symbol...
                head_rule->prev->next = new_rule2;
                new_rule2->prev = head_rule->prev;
                head_rule->prev = new_rule2;
                new_rule2->next = head_rule;
            }
        }
    } // end of while(1) loop
    // printf("amount_bytes: %d", amount_bytes);
    fflush(out); // ensure no output remains buffered in memory.
    return amount_bytes; // file does not follow format
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