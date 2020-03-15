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
#include <math.h>

size_t calculateSize(size_t size) {
    // Header size, Footer, Next & Prev Ptrs, Requested Size (alignment = multiple of 64)
    size_t size_no_padding = sizeof(sf_header) + sizeof(sf_footer) + (2 * sizeof(sf_block *)) + size;
    int pre_padding = (size_no_padding + 32) / 64; // maintain proper alignment
    size_t blocksize = round(pre_padding) * 64; // now includes alignment
    return blocksize;
}

void find_freelist(size_t blocksize) {

}

// -----------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

int first_allocation = 0;
void *sf_malloc(size_t size) {

    // Segregated by size class

    // if size is 0, return NULL
    if (size == 0) return NULL; // without setting sf_errno
    else {
        // CHECK IF FIRST ALLOCATION REQUEST
        if (first_allocation == 0) {
            sf_mem_init();
            sf_mem_grow();
            first_allocation = 1;
        }

        size_t blocksize = calculateSize(size);
        find_freelist(blocksize); // Determine smallest free list to satisfy a request of that size

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
