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

    int verify_compress = global_options & 2;
    if (verify_compress == 2) {
        compress(stdin, stdout, 1024);
    }

    return EXIT_SUCCESS;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
