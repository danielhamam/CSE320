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
size_t insertFreeBlock(int remainder);
int findIndex(int size);

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
            // return payloadPtr;
            first_allocation = 1;
        }

        size_t blocksize = calculateSize(size);
        payloadPtr = find_freelist(blocksize); // Determine smallest free list to satisfy a request of that size
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

    // THE WILDERNESS BLOCK ("inserted into the free list as a single block")

    startHeap += 64; // 6 unused rows of padding x 8 bytes each (block is 64 bytes) --> 48 + 16 (links)

    sf_block *wilderness = (sf_block *) startHeap;
    int space = (sf_mem_end() - 16) - startHeap;
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
    epilogue->header = (size_t) 1; // allocated is 1
}

size_t calculateSize(size_t size) {
    // Header size, Footer, Next & Prev Ptrs, Requested Size (alignment = multiple of 64)
    if (size < 32) {
        return 64; // if below this range, automatically aligned to 64
    }
    else {
        int adjustedSize = sizeof(sf_header) + ( (int) size);
        int blocksize = (( adjustedSize + 32) / 64) * 64;
        debug("ADJUSTEDSIZE: %d", adjustedSize);
        debug("BLOCKSIZE: %d", blocksize);
        return blocksize;
    }
}

void *find_freelist(size_t requestSize) {
    int index =  requestSize / 64;
    int isWilderness = 0;
    debug("REQUEST SIZE: %d", (int) requestSize);
    debug("INDEX: %d", index);
    // First, we start with the first size class from int index

    while (index <= (NUM_FREE_LISTS - 1)) {

        sf_block sentinelBlock = sf_free_list_heads[index]; // dummy node connecting first and last
        sf_block *targetBlock = sentinelBlock.body.links.next;
        debug("index %d ", index);
        int count = 0;

        // Check through that free list for appropriate size
        while (targetBlock != &sf_free_list_heads[index]) { // end of free list equal to &sentinelBlock
            count++;
            debug("count : %d", count);
            // check if you found
            int foundSize = targetBlock->header & BLOCK_SIZE_MASK; // get size excluding last 2 bits (which check allocation)

            if (foundSize == requestSize) {
                // found the correct EXACT block size

                // Adding the allocated bit
                sf_block *allocatedBlock = targetBlock;
                allocatedBlock->header = (allocatedBlock->header | 1);

                // REMOVE FREE LIST FROM ARRAY
                sf_block *nextFreeBlock = allocatedBlock->body.links.next;
                sf_block *prevFreeBlock = allocatedBlock->body.links.prev;
                nextFreeBlock->body.links.prev = allocatedBlock->body.links.prev; // REMOVE FROM FREE LIST
                prevFreeBlock->body.links.next = allocatedBlock->body.links.next;
                allocatedBlock->body.links.next = NULL;
                allocatedBlock->body.links.prev = NULL;

                // Access next block's contents
                sf_block *nextBlock = allocatedBlock + foundSize;
                nextBlock->prev_footer = ( (nextBlock->prev_footer) | 2);

                void *payloadPtr = allocatedBlock->body.payload;
                debug("Success1");
                return payloadPtr;

            }
            else if (foundSize > requestSize) {

                // 1. SPLITTING
                // We are at the header of the wilderness block
                // The two pointers in the free block are in a union with the payload, so share same space
                // (lower part = allocation request, upper part = free block)

                if (index == 9) isWilderness = 1; // we're using the wilderness block
                size_t remainder = (size_t) (foundSize - requestSize);

                // Allocated Block
                void *block_address = targetBlock;
                sf_block *newAllocatedBlock = (sf_block*) block_address;

                // Create allocated block header, check if previous block is allocated
                int isAllocated_prev = (targetBlock->prev_footer & PREV_BLOCK_ALLOCATED);

                if (isAllocated_prev == 0) newAllocatedBlock->header = (size_t) requestSize | (1); // for allocated (1)
                else newAllocatedBlock->header = (size_t) requestSize | (3); // for allocated (1)

                sf_block *nextBlock = newAllocatedBlock->body.links.next;
                sf_block *prevBlock = newAllocatedBlock->body.links.prev;

                nextBlock->body.links.prev = newAllocatedBlock->body.links.prev;
                prevBlock->body.links.next = newAllocatedBlock->body.links.next;

                // Free Block's Header
                sf_block *newFreeBlock = (sf_block*) (block_address + requestSize); // start of free block (upper)
                newFreeBlock->header= (size_t) (remainder | 2); // prev_allocated = 1, allocated = 0.

                // Free Block's Footer
                sf_block *newFreeBlock_nextBlock = (sf_block*) (block_address + foundSize);
                newFreeBlock_nextBlock->prev_footer = (size_t) (remainder | 2);

                // return NULL;

                // ---------------------------------------------------------------------------

                newAllocatedBlock->body.links.next = NULL;
                newAllocatedBlock->body.links.prev = NULL;


                // LETS FIX THE LINKED LISTS:

                if (isWilderness == 0) {
                    int freeBlockIndex = findIndex( (int) remainder);
                    newFreeBlock->body.links.prev = sf_free_list_heads[freeBlockIndex].body.links.prev;
                    newFreeBlock->body.links.next = &sf_free_list_heads[freeBlockIndex];
                    (newFreeBlock->body.links.next)->body.links.prev = newFreeBlock;
                    sf_free_list_heads[freeBlockIndex].body.links.next = newFreeBlock;
                }
                else {
                    // It's the wilderness block (if you split WB, free block becomes new wilderness block. ALWAYS LAST LIST)
                    nextBlock->body.links.prev = newFreeBlock;
                    prevBlock->body.links.next = newFreeBlock;
                    newFreeBlock->body.links.prev = prevBlock;
                    newFreeBlock->body.links.next = nextBlock;
                }

                // ---------------------------------------------------------------------------

                char *payloadPtr = newAllocatedBlock->body.payload;
                debug("Success2");
                return payloadPtr;

            }
            targetBlock = targetBlock->body.links.next; // get next
        } // end of inner while loop
        index++;
    } // end of first while loop

    // if you make it here, then no block was big enough!
    // must use sf_mem_grow to request more memory (if not possible, sf_errno = ENOmem, and return NULL)
    return NULL;
}

int findIndex(int size) {

    // M = 64
    // M = 0
    // 2M = 1
    // 3M = 2
    // 3M --> 5M = 3
    // 5M --> 8M = 4
    // 8M --> 13M = 5
    // 13M --> 21M = 6
    // 21M --> 34M = 7
    // > 34M = 8

    if (size == 64) return 0; // M
    if (size == 128) return 1; // 2M
    if (size == 192) return 2; // 3M
    if (size >= 193 && size <= 320) return 3; // 3M --> 5M
    if (size >= 321 && size <= 512) return 4; // 5M --> 8M
    if (size >= 513 && size <= 832) return 5; // 8M --> 13M
    if (size >= 833 && size <= 1344) return 6; // 13M --> 21M
    if (size >= 1345 && size <= 2176) return 7; // 21M --> 34M
    else return 8;
    // 9  holds the wilderness block

}
