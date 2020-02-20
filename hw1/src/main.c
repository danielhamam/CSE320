#include <stdio.h>
#include <stdlib.h>

#include "const.h"
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

int main(int argc, char **argv)
{

    if (validargs(argc, argv))
        USAGE(*argv, EXIT_FAILURE);
    debug("Options: 0x%x", global_options);
    if (global_options & 1)
        USAGE(*argv, EXIT_SUCCESS);

    int verify_decompress = global_options & 4;
    if (verify_decompress == 4) {
        decompress(stdin, stdout);
    }

// -------------------------------------------------------------------
    int verify_compress = global_options & 2;
    char **find_block = argv;
    find_block = find_block + 3;
    char *block_ptr = *find_block;
    int bsize = 0;
    while (*block_ptr != '\0') {
        char c = *block_ptr;
        if (c >= '0' && c <= '9') {
            int val = c - 48; // SUBTRACT ASCII VALUES.
            bsize = bsize * 10;
            bsize = bsize + val;
        } else {
            return -1;
        }
        block_ptr++;
    }
// -------------------------------------------------------------------

    if (verify_compress == 2) {
        compress(stdin, stdout, bsize);
    }

    return EXIT_SUCCESS;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
