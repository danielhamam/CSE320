/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "sfmm.h"

void *sf_malloc(size_t size) {

    // Segregated by size class

    // if size is 0, return NULL
    if (size == 0) return NULL; // without setting sf_errno
    else {
        // Determine overall size, adding header size, padding. (alignment = multiple of 64)
        size_t size_no_padding = sizeof(sf_header) + sizeof(sf_footer) + size;
        size_t size_padding = 64 * ( size_no_padding / 64);

    }

    return NULL;
}

void sf_free(void *pp) {
    return;
}

void *sf_realloc(void *pp, size_t rsize) {
    return NULL;
}

void *sf_memalign(size_t size, size_t align) {
    return NULL;
}
