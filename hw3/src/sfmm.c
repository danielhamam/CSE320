/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "debug.h"
#include "sfmm.h"

size_t calculateSize(size_t size) {
    // Header size, Footer, Next & Prev Ptrs, Requested Size (alignment = multiple of 64)
    size_t size_no_padding = sizeof(sf_header) + sizeof(sf_footer) + (2 * sizeof(sf_block *)) + size;
    int pre_padding = (size_no_padding + 32) / 64; // maintain proper alignment
    size_t blocksize = round(pre_padding) * 64; // now includes alignment
    return blocksize;
}

void find_freelist(size_t blocksize) {
    int index =  blocksize / 64;

    // First, we start with the first size class from int index
    while (index != (NUM_FREE_LISTS - 1)) {

        sf_block sentinelBlock = sf_free_list_heads[index]; // dummy node connecting first and last
        sf_block *targetBlock = sentinelBlock.body.links.next;

        // Check through that free list for appropriate size
        while (targetBlock != &sentinelBlock) { // end of free list equal to &sentinelBlock

            // check if you found
            int amountSize = targetBlock->header & 0x11111100; // get size excluding last 2 bits (which check allocation)
            if (amountSize >= blocksize) {
                // found the correct block size
                // 1. split it if required
                // 2. lower part = allocation request, upper part = remainder

            targetBlock = targetBlock->body.links.next; // get next
            }
        } // end of inner while loop
        index++;
    } // end of first while loop

    // if you make it here, then no block was big enough!
    // must use sf_mem_grow to request more memory (if not possible, sf_errno = ENOmem, and return NULL)

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
