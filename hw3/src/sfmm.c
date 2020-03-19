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

void initialize_heap();
size_t calculateSize(size_t size);
void *find_freelist(size_t blocksize);

int first_allocation = 0;
void *sf_malloc(size_t size) {

    // Segregated by size class
    void *payloadPtr = NULL;

    // if size is 0, return NULL
    if (size <= 0) return NULL; // without setting sf_errno
    else {
        // CHECK IF FIRST ALLOCATION REQUEST
        if (first_allocation == 0) {
            initialize_heap();
            return payloadPtr;
            first_allocation = 1;
        }

        size_t blocksize = calculateSize(size);
        payloadPtr = find_freelist(blocksize); // Determine smallest free list to satisfy a request of that size
        return payloadPtr;
    }
    return payloadPtr;
}

void sf_free(void *pp) {

    // find address of the block
    sf_header *blockHead = pp - sizeof(sf_header);
    int allocated = (*blockHead) & THIS_BLOCK_ALLOCATED;
    if (!allocated || pp == NULL) abort(); // "if ptr is invalid, call abort()"

    (*blockHead) = (*blockHead) & 0xfffffffe; // clear allocated bit --> free bit
}

void *sf_realloc(void *pp, size_t rsize) {
    return NULL;
}

void *sf_memalign(size_t size, size_t align) {
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

//                                                  HELPER FUNCTIONS

// -----------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------


void initialize_heap() {
    sf_mem_init();
    void *startHeap = sf_mem_grow(); // beginning of heap
    startHeap += 48; // 56 bytes (7 rows x 8), but 8 bytes for footer so 48. ("overlaps the last 8 bytes of padding")

    // INITIALIZE THE PROLOGUE BLOCK
    sf_block *prologue = (sf_block *) startHeap;
    prologue->header = (size_t) 67; // 1000011 allocated is 1, prev_allocated is 1
    startHeap += 64; // 6 unused rows of padding x 8 bytes each (block is 64 bytes) --> 48 + 16 (links)
    // Set the footer since we're at that address:
    sf_block *prologue2 = (sf_block *) startHeap;
    prologue2->prev_footer = (size_t) 67; // belongs to prologue (this is the prologue footer)    PROLOGUE HEADER = FOOTER (CONTENTS)

    // THE WILDERNESS BLOCK ("inserted into the free list as a single block")
    // startHeap += 8; // links/header

    sf_block *wilderness = (sf_block *) startHeap;
    int space = ((sf_mem_end() - 16) - startHeap);
    debug("SPACE: %d", space);
    wilderness->prev_footer = (size_t) 67;
    wilderness->header = (size_t) space;

    sf_free_list_heads[NUM_FREE_LISTS - 1].body.links.next = (wilderness);
    sf_free_list_heads[NUM_FREE_LISTS - 1].body.links.prev = (wilderness);
    wilderness->body.links.next = &sf_free_list_heads[NUM_FREE_LISTS - 1];
    wilderness->body.links.prev = &sf_free_list_heads[NUM_FREE_LISTS - 1];

    // initialize the free list
    int index_tracker = 0;
    while (index_tracker != 9) {
        // if equal to address of itself, it is empty list.
        sf_free_list_heads[index_tracker].body.links.next = &sf_free_list_heads[index_tracker];
        sf_free_list_heads[index_tracker].body.links.prev = &sf_free_list_heads[index_tracker];
        index_tracker++;
    }

    // INITIALIZE THE EPILOGUE BLOCK
    void *endHeap = sf_mem_end() - 16; // because 8 is the footer
    sf_block *epilogue = (sf_block *) endHeap;
    epilogue->prev_footer = (size_t) space;
    epilogue->header = (size_t) 1; // 1000011 allocated is 1, prev_allocated is 1
}

size_t calculateSize(size_t size) {
    // Header size, Footer, Next & Prev Ptrs, Requested Size (alignment = multiple of 64)
    size_t size_no_padding = sizeof(sf_header) + sizeof(sf_footer) + size;
    int pre_padding = (size_no_padding + 32) / 64; // maintain proper alignment
    size_t blocksize = round(pre_padding) * 64; // now includes alignment
    return blocksize;
}

void *find_freelist(size_t blocksize) {
    int index =  blocksize / 64;

    // First, we start with the first size class from int index
    while (index != (NUM_FREE_LISTS - 1)) {

        sf_block sentinelBlock = sf_free_list_heads[index]; // dummy node connecting first and last
        sf_block *targetBlock = sentinelBlock.body.links.next;

        // Check through that free list for appropriate size
        while (targetBlock != &sentinelBlock) { // end of free list equal to &sentinelBlock

            // check if you found
            int amountSize = targetBlock->header & BLOCK_SIZE_MASK; // get size excluding last 2 bits (which check allocation)
            if (amountSize == blocksize) {
                // found the correct EXACT block size
                // remove free list from free list
                void *payloadPtr = targetBlock->body.payload;
                return payloadPtr;
            }
            else if (amountSize > blocksize) {
                // 1. split it
                  // make a new block with that size?
                // 2. lower part = allocation request, upper part = remainder
            }
            targetBlock = targetBlock->body.links.next; // get next
        } // end of inner while loop
        index++;
    } // end of first while loop

    // if you make it here, then no block was big enough!
    // must use sf_mem_grow to request more memory (if not possible, sf_errno = ENOmem, and return NULL)
    return NULL;
}
