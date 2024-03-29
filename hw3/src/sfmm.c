/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include "debug.h"
#include "sfmm.h"

void initialize_heap();
size_t calculateSize(size_t size);
void *find_freelist(size_t blocksize);
size_t insertFreeBlock(int remainder);
int findIndex(int size);
void *createFreeBlock(size_t request);
int checkPointer(void *pointer);
void *coalesce(void *pointer);
int checkPowerofTwo(size_t number);
size_t calculateSizeAlign(size_t size, size_t align);

int first_allocation = 0;
void *sf_malloc(size_t size) {

    // Segregated by size class
    void *blockVoidPtr = NULL;

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
        blockVoidPtr = find_freelist(blocksize); // Determine smallest free list to satisfy a request of that size
    }
    if (blockVoidPtr == NULL) return NULL;
    else {
        sf_block *targetBlock = (sf_block *) blockVoidPtr;
        // debug("TARGET BLOCK HEADER SIZE ---> %d ", (int) targetBlock->header & BLOCK_SIZE_MASK);
        return targetBlock->body.payload;
    }
}

void sf_free(void *pp) {

    // First, Check If PP (pointer) Is Valid (aka, if it's allocated)
    int ptrNum = checkPointer(pp);
    if (ptrNum == -1) abort();
    // Else, we can continue. The pointer is valid.

    void *ptr_Start = pp - 16; // goes to prev_footer
    coalesce(ptr_Start);
}

void *sf_realloc(void *pp, size_t rsize) {

    int ptrNum = checkPointer(pp);
    int edited_rsize = calculateSize(rsize);
    if (ptrNum == -1) abort();
    // Else, we can continue. The pointer is valid.

    void *ptr_Start = pp - 16;
    sf_block *ptrBlock = (sf_block *) ptr_Start;

    // if pointer is valid, but size is 0.
    if ((ptrBlock->header & BLOCK_SIZE_MASK) == 0) {
        sf_free(pp);
        return NULL;
    }

    if (edited_rsize == (ptrBlock->header & BLOCK_SIZE_MASK)) return pp;
    if (edited_rsize > (ptrBlock->header & BLOCK_SIZE_MASK)) {

        void *newPP = sf_malloc(rsize);

        // If sf_malloc returns null, return NULL:
        if (newPP == NULL) return NULL;

        // Copies contents of pp into newPP (second into first)
        memcpy(newPP, pp, (ptrBlock->header & BLOCK_SIZE_MASK));

        // Calling free on block given by the client
        sf_free(pp);

        // Return block given by sf_malloc
        return newPP;
    }

    else {
        // rsize is smaller, so either split or update with smaller header (if splinter)
        size_t adjustedSize = calculateSize(rsize);
        size_t ptrBlock_size = ptrBlock->header & BLOCK_SIZE_MASK;
        // debug("SMALLER");
        if (adjustedSize == ptrBlock_size) {
            // DO NOT SPLIT
            // debug("Dont split");
            return pp;

        }
        else {
            // WE MUST SPLIT (rsize is smaller, we have to split the ptrBlock)
            // two blocks --> one free with remainder, one with allocated r size
            size_t remainder = ptrBlock_size - adjustedSize;

            // Change ALLOCATED header's contents
            int initialPrevAlloc = ptrBlock->header & PREV_BLOCK_ALLOCATED;
            ptrBlock->header = (size_t) (adjustedSize | 1 | initialPrevAlloc); // Doesn't have a footer because allocated

            // Access Free block
            void *freeBlockAddr = ptr_Start + adjustedSize;
            sf_block *freeBlock = (sf_block *) freeBlockAddr;

            // Change free block's header and footer
            freeBlock->header = (remainder | 2); // 10 because previous is allocated
            void *freeBlock_footerAddr = freeBlockAddr + remainder; // remainder is size of block
            sf_block *afterFreeBlock = (sf_block *) freeBlock_footerAddr;
            afterFreeBlock->prev_footer = (remainder | 2);

            // Take out prev allocated of next block (FOR ITS HEADER AND FOOTER)
            afterFreeBlock->header = (afterFreeBlock->header) - 2;

            // Only change footer if its a free block
            if ( (afterFreeBlock->header & THIS_BLOCK_ALLOCATED) != 1) {
                void *footer_afterFreeBlockAddr = freeBlock_footerAddr + (int) (afterFreeBlock->header & BLOCK_SIZE_MASK);
                sf_block *footer_afterFreeBlock = (sf_block *) footer_afterFreeBlockAddr;
                footer_afterFreeBlock->prev_footer = (afterFreeBlock->header);
            }

            // Remove links from allocated block (has no links, just precaution)
            ptrBlock->body.links.prev = NULL;
            ptrBlock->body.links.next = NULL;

            // Links will be added when coalescing

            // Coalesce the free block created
            coalesce(freeBlockAddr);

            // Return a pointer to payload of smaller block
            return ptrBlock->body.payload;

        }


    }

    return NULL;
}

void *sf_memalign(size_t size, size_t align) {

    // Check if requested alignment is at least as large as the minimum block size
    if (align < 64) { sf_errno = EINVAL; return NULL; }

    // Check if requested alignment is a power of two
    int resultPower = checkPowerofTwo(align);
    if (resultPower == -1) { sf_errno = EINVAL; return NULL; }

    size_t blocksize = calculateSizeAlign(size, align);

    void *mallocPtr = sf_malloc(blocksize);

    void *startBlock = mallocPtr - 16;
    sf_block *mallocBlock = (sf_block *) startBlock;
    int mallocBlock_size = mallocBlock->header & BLOCK_SIZE_MASK;

    intptr_t payload_Address = (intptr_t) &(mallocBlock->body.payload); // To get unsigned num for hex pointer
    if (payload_Address % align == 0) {

        // (ALIGNED) Don't do anything
        return &(mallocBlock->body.payload);

    }
    else {

        // (NOT ALIGNED) split, free previous block, realloc, return

        int remainder;
        int pushUp;
        int allocatedBlockSize;

            // Re-allocate the size
            remainder = (int) payload_Address % align; // ptrBlock_address needs ptrBlock_address + remainder to be allocated

            // (payload_address + pushUp) % align = 0
            // 500 mod 216 = 68. If you do 216 - 68, then you push 500 up by 148.

            pushUp = align - remainder;
            // pushUp = pushUp - 16; // because start of block is 16 before
            allocatedBlockSize = mallocBlock_size - pushUp;
            // debug("PUSHUP --> %d ", pushUp);

        // First part = free block, Second part = allocated block

        // Let's set the free block
        int prevAllocated_Original = mallocBlock->header & PREV_BLOCK_ALLOCATED;
        void *free_block_start = startBlock;
        sf_block *new_freeBlock = (sf_block *) free_block_start;
        new_freeBlock->header = (size_t) pushUp | prevAllocated_Original;

        // Let's set the allocated block
        void *new_allocatedBlockAddr = startBlock + pushUp;
        sf_block *new_allocatedBlock = (sf_block *) new_allocatedBlockAddr;
        new_allocatedBlock->prev_footer = (new_freeBlock->header);
        new_allocatedBlock->header = allocatedBlockSize | 1; // 1 because it's allocated. Previous block is a free block.

        coalesce(free_block_start);

        // NOW, IF THAT SIZE IS TOO BIG, REALLOC TO REQUESTED SIZE:
        if (allocatedBlockSize > size) {
            // debug("SECOND ---> REALLOC");
            void *new_mallocPtr = sf_realloc( (new_allocatedBlockAddr + 16), size); // realloc handles the free blocks and all that
            return new_mallocPtr;
        }
        // Else:

        return &(new_allocatedBlock->body.payload);
    }
}

// -----------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

//                                                  HELPER FUNCTIONS

// -----------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------
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
    // debug("SPACE: %d", space);
    wilderness->prev_footer = (size_t) 67;
    wilderness->header = (size_t) space | 2; // prev_alloc is 1 because of prologue

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
    epilogue->prev_footer = (size_t) space | 2; // prev_alloc is 1 because of prologue
    epilogue->header = (size_t) 1; // allocated is 1
}

size_t calculateSize(size_t requestSize) {
    // Header size, Footer, Next & Prev Ptrs, Requested Size (alignment = multiple of 64)
    requestSize += 8;
    if (requestSize % 64 == 0) return requestSize;
    size_t result = requestSize + (64 - (requestSize % 64));
    return result;
}

void *find_freelist(size_t requestSize) {

    // int index = requestSize / 64;
    int index = findIndex(requestSize);
            // debug("INDEX: %d ", index);
    int isWilderness = 0;

    // Let's check if the index is valid:
    if (index < 0) {
        sf_errno = ENOMEM;
        return NULL;
    }

    // First, we start with the first size class from int index

    while (index <= (NUM_FREE_LISTS - 1)) {

        sf_block sentinelBlock = sf_free_list_heads[index]; // dummy node connecting first and last
        sf_block *targetBlock = sentinelBlock.body.links.next;
        void *block_address = targetBlock;


        // Check through that free list for appropriate size
        while (targetBlock != &sf_free_list_heads[index]) { // end of free list equal to &sentinelBlock
            // check if you found
            int foundSize = targetBlock->header & BLOCK_SIZE_MASK; // get size excluding last 2 bits (which check allocation)

            if (foundSize == requestSize) {
                // found the correct EXACT block size

                // Adding the allocated bit
                sf_block *allocatedBlock = (sf_block *) block_address;

                // check if previous block is allocated

                int InitialPreviousAlloc = targetBlock->header & PREV_BLOCK_ALLOCATED;
                allocatedBlock->header = (size_t) requestSize | (1) | InitialPreviousAlloc; // for allocated (1)

                // Change next block's PAL and remove footer
                void *nextBlockAddr = block_address + foundSize;
                sf_block *nextBlock = (sf_block *) nextBlockAddr;
                nextBlock->prev_footer = 0;
                int nextBlockPrevAlloc = nextBlock->header & PREV_BLOCK_ALLOCATED;
                if (nextBlockPrevAlloc == 0) nextBlock->header += 2;

                // REMOVE FREE LIST FROM ARRAY
                sf_block *nextFreeBlock = allocatedBlock->body.links.next;
                sf_block *prevFreeBlock = allocatedBlock->body.links.prev;
                nextFreeBlock->body.links.prev = allocatedBlock->body.links.prev; // REMOVE FROM FREE LIST
                prevFreeBlock->body.links.next = allocatedBlock->body.links.next;
                allocatedBlock->body.links.next = NULL;
                allocatedBlock->body.links.prev = NULL;

                // allocated Blocks don't have footers

                return block_address;

            }
            else if (foundSize > requestSize) {
                // 1. SPLITTING
                // We are at the header of the wilderness block
                // The two pointers in the free block are in a union with the payload, so share same space
                // (lower part = allocation request, upper part = free block)

                if (index == 9) isWilderness = 1; // we're using the wilderness block
                size_t remainder = (size_t) (foundSize - requestSize);

                // Allocated Block
                sf_block *newAllocatedBlock = (sf_block*) block_address;

                // Create allocated block header, check if previous block is allocated

                int InitialPreviousAlloc = targetBlock->header & PREV_BLOCK_ALLOCATED;
                newAllocatedBlock->header = (size_t) requestSize | (1) | InitialPreviousAlloc; // for allocated (1)

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

                // char *payloadPtr = newAllocatedBlock->body.payload;
                return block_address;

            }
            targetBlock = targetBlock->body.links.next; // get next
        } // end of inner while loop
        index++;
    } // end of first while loop

    // if you make it here, then no block was big enough!
    // must use sf_mem_grow to request more memory (if not possible, sf_errno = ENOmem, and return NULL)

    void *newBlock = createFreeBlock(requestSize);
    if (newBlock == NULL) return NULL;
    else {
        return newBlock;
    }

}

// Function responsible to have another free block. According to the document, after each call to sf_mem_grow, I should
// coalesce the page with the wilderness block (if any) preceding it. This allows me to build blocks more than one page. This
// new wilderness block is placed at the beginning of the last freelist

void *createFreeBlock(size_t requestSize) {

    void *wilderness;
    size_t newFreeBlock_Size;
    size_t SavedPrevFooter;
    void *endHeap = sf_mem_end() - 16; // Starts at footer of WB
    sf_block *epilogue = (sf_block *) endHeap;

    int initialPrevAlloc = epilogue->prev_footer & PREV_BLOCK_ALLOCATED;
    // debug("INITIALPREVALLOC ----> %d ", initialPrevAlloc);

    if ( (epilogue->prev_footer & PREV_BLOCK_ALLOCATED) == 1 ) {
        // Previous Block is Allocated (WB has 0 FREE SPACE)
        newFreeBlock_Size = 0;
        wilderness = sf_mem_end() - 8; // start address of wilderness block
    }
    else {
        // Previous Block is not Allocated (FREE BLOCK)

        // Let's find the start address header
        newFreeBlock_Size = epilogue->prev_footer & BLOCK_SIZE_MASK; // prev footer of epilogue holds prev block's size (held initally WB size)

        // Let's access previous block (WB)
        void *FirstWilderness_Address = endHeap - newFreeBlock_Size;
        sf_block * initial_Wilderness = (sf_block *) FirstWilderness_Address;
        SavedPrevFooter = initial_Wilderness->prev_footer;
        // SAVED THE PREV_FOOTER. PUT THIS BACK INTO NEW WB

        wilderness = (sf_mem_end() - 16) - newFreeBlock_Size; // start address of FIRST wildernes block

        // Reset the Epilogue
        // epilogue->prev_footer = 0;
        // epilogue->header = 0;

    }

    while (requestSize > newFreeBlock_Size) {

        if (sf_mem_grow() != NULL) newFreeBlock_Size += 4096;
        else {

            // Add the memory
            sf_block *wildernessBlock = (sf_block *) wilderness;

            // Edit wilderness block's header
            wildernessBlock->prev_footer = SavedPrevFooter;
            wildernessBlock->header = (size_t) newFreeBlock_Size | initialPrevAlloc;

            // Edit wilderness block's footer (epilogue)
            void *newEpilogue_address = wilderness + newFreeBlock_Size;
            sf_block *newEpilogue = (sf_block *) newEpilogue_address;
            newEpilogue->prev_footer = (size_t) newFreeBlock_Size | initialPrevAlloc;
            newEpilogue->header = (size_t) 1;

            // Can not access more sf_mem_grow()
            sf_errno = ENOMEM;
            return NULL;
        }
    } // end of while loop
    // If we make it here, we assume sf_mem_grow was able to allocate memory for all requested space
        // "Coalesce newly allocated page with WB preceding it"
        // "Insert new WB at beginning of last free list"
        // "Do not coalesce with prologue, epilogue, or past the beginning or end of heap"

    sf_block *wildernessBlock = (sf_block *) wilderness;

    // debug("INITIAL WILDERNESS BLOCK ---> %d ", (int) wildernessBlock->header);

    size_t new_WildernessSize = (size_t) newFreeBlock_Size; // new size of wilderness block

    // Edit wilderness block's header (NEW HEADER)
    wildernessBlock->prev_footer = SavedPrevFooter; // for the prologue block
    wildernessBlock->header = (size_t) new_WildernessSize | initialPrevAlloc;

    // Edit wilderness block's footer (NEW FOOTER) --> THIS IS THE EPILOGUE
    void *newEpilogue_address = wilderness + new_WildernessSize;
    sf_block *newEpilogue = (sf_block *) newEpilogue_address;
    newEpilogue->prev_footer = (size_t) new_WildernessSize | initialPrevAlloc;
    newEpilogue->header = (size_t) 1;
    // newEpilogue address is correct (points to epilogue)

    // ALLOCATION STEP:
    // ---------------------------------------------------------------------------------
    // Now, ALLOCATE the block with the amount requested. Should split WB if needed
    // ---------------------------------------------------------------------------------

    if (new_WildernessSize == requestSize) {
        // dont split the wilderness block

        // Adding the allocated bit
        wildernessBlock->header = (requestSize | 1 | initialPrevAlloc); // 1 for prev_allocated, 1 for allocated

        // REMOVE FREE LIST FROM ARRAY
        sf_block *nextFreeBlock = wildernessBlock->body.links.next;
        sf_block *prevFreeBlock = wildernessBlock->body.links.prev;
        nextFreeBlock->body.links.prev = wildernessBlock->body.links.prev; // REMOVE FROM FREE LIST
        prevFreeBlock->body.links.next = wildernessBlock->body.links.next;
        wildernessBlock->body.links.next = NULL;
        wildernessBlock->body.links.prev = NULL;

        // Access next block's contents
        sf_block *nextBlock = wilderness + requestSize;
        nextBlock->prev_footer = (requestSize | 1 | initialPrevAlloc); // this would be the epilogue

        // Make next block also PAL = 1 (the targetBlock was a free block)
        nextBlock->header = (nextBlock->header) | 2; // 10 for PAL

        // IF IT'S A FREE BLOCK, UPDATE FOOTER TOO
        if ( (nextBlock->header & THIS_BLOCK_ALLOCATED) == 0) {
            size_t nextBlockSize = (size_t) nextBlock->header & BLOCK_SIZE_MASK;
            void *nextBlock_footerAddr = (wilderness + requestSize) + (nextBlockSize);
            sf_block *nextBlockfooter = (sf_block *) nextBlock_footerAddr;
            nextBlockfooter->prev_footer = (nextBlock->header);
        }

        return wilderness;
    }
    else if (new_WildernessSize > requestSize) {

        // debug("GREATER THAN");

        // split the wilderness block
        size_t remainder = (size_t) (new_WildernessSize - requestSize);

        // Allocated Block
        sf_block *newAllocatedBlock = (sf_block*) wilderness; // wilderness is the start of the WB block

        // Create allocated block header, check if previous block is allocated
        // In the beginning --> int initialPrevAlloc = epilogue->prev_footer & PREV_BLOCK_ALLOCATED;

        if (initialPrevAlloc == 0) newAllocatedBlock->header = (size_t) requestSize | (1); // for allocated (1)
        else newAllocatedBlock->header = (size_t) requestSize | (3); // for prev_allocated (1) and allocated(1)

        sf_block *nextBlock = newAllocatedBlock->body.links.next;
        sf_block *prevBlock = newAllocatedBlock->body.links.prev;

        nextBlock->body.links.prev = newAllocatedBlock->body.links.prev;
        prevBlock->body.links.next = newAllocatedBlock->body.links.next;

        // Free Block's Header
        sf_block *newFreeBlock = (sf_block*) (wilderness + requestSize); // start of free block (upper)
        newFreeBlock->header = (size_t) (remainder | 2); // prev_allocated = 1, allocated = 0.

        // Free Block's Footer
        sf_block *newFreeBlock_nextBlock = (sf_block*) (wilderness + new_WildernessSize);
        newFreeBlock_nextBlock->prev_footer = (size_t) (remainder | 2);

        newAllocatedBlock->body.links.next = NULL;
        newAllocatedBlock->body.links.prev = NULL;

        // It's the wilderness block (if you split WB, free block becomes new wilderness block. ALWAYS LAST LIST)

        nextBlock->body.links.prev = newFreeBlock;
        prevBlock->body.links.next = newFreeBlock;
        newFreeBlock->body.links.prev = prevBlock;
        newFreeBlock->body.links.next = nextBlock;

        return wilderness;

    }
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

// -1 = failure, success = 0
int checkPointer(void *pointer) {

    // pointer is a pointer to the payload of the malloc

    // Check if NULL
    if (pointer == NULL) return -1;

    // Check if NOT ALLOCATED
    void *ptr_address = pointer - 16;
    sf_block *ptrBlock = (sf_block *) ptr_address; // Now points to the start of the block (with prev_footer)
    size_t block_size = ptrBlock->header & BLOCK_SIZE_MASK;
    int allocated = (ptrBlock->header & THIS_BLOCK_ALLOCATED);
    if (allocated == 0) return -1; // "if ptr is invalid, call abort()"

    // Check if POINTER ADDRESS is aligned with 64 boundary (NOT BLOCK)
    uintptr_t ptrBlock_Address = (uintptr_t)&ptrBlock->body.payload;
    if (ptrBlock_Address % 64 != 0) return -1;

    // Check if "the header of the block = before end of the prologue"
    // Check if "the footer of the block = after beginning of epilogue"
    if ( (pointer + 8) < sf_mem_start() + 64 + 56|| (pointer - 8 + block_size) > sf_mem_end() - 8) return -1;

    // Check if prev_alloc is 0 but alloc of prev block is 1
    int markedPrevAllocated = ptrBlock->header & PREV_BLOCK_ALLOCATED;
    int realAllocated = ptrBlock->prev_footer & THIS_BLOCK_ALLOCATED;
    if (markedPrevAllocated == 0 && realAllocated == 1) return -1;

    return 0;

}

void *coalesce(void *pointer) {

    sf_block *ptrBlock = (sf_block *) pointer;
    size_t ptrBlock_size = (size_t) ptrBlock->header & BLOCK_SIZE_MASK;

    // Boolean markers for allocation
    int nextBlockAllocated = 0;
    int prevBlockAllocated = 0;

    // NEXT BLOCK (AFTER POINTER BLOCK)
    void *ptrBlock_footer = pointer + ptrBlock_size;
    sf_block *nextBlock = (sf_block*) ptrBlock_footer;
    if ( (nextBlock->header & THIS_BLOCK_ALLOCATED) == 1) nextBlockAllocated = 1;

    // PREVIOUS BLOCK (BEFORE POINTER BLOCK)

    // Problem: allocated block DOES NOT have a footer
    // Assuming that there is a prev_footer (it is allocated)
    size_t prevBlock_size = ptrBlock->prev_footer & BLOCK_SIZE_MASK;
    void *prevBlock_address = pointer - prevBlock_size;
    sf_block *prevBlock = (sf_block*) prevBlock_address; // goes to prev_footer of the previous block
    // if ( (prevBlock->header & THIS_BLOCK_ALLOCATED) == 1) prevBlockAllocated = 1;
    if ( (ptrBlock->header & PREV_BLOCK_ALLOCATED) == 2) prevBlockAllocated = 1;

    void *newBlock_address;
    sf_block *newBlock;

    // Combine:

    // debug("PREVBLOCKALLOCATED: %d ", (int) prevBlockAllocated );
    // debug("NEXTBLOCKALLOCATED: %d \n", nextBlockAllocated);

    // If nextBlock is FREE but prevBlock is ALLOCATED
    if ( (nextBlockAllocated != 1) && (prevBlockAllocated == 1) ) {

        // debug("TESTINGTESTINGTESTING");
        // Remove Next Block's LINKS:
        sf_block *nextFreeBlock_next = nextBlock->body.links.next;
        sf_block *prevFreeBlock_next = nextBlock->body.links.prev;
        nextFreeBlock_next->body.links.prev = nextBlock->body.links.prev; // REMOVE FROM FREE LIST
        prevFreeBlock_next->body.links.next = nextBlock->body.links.next;
        nextBlock->body.links.next = NULL;
        nextBlock->body.links.prev = NULL;

        // debug("TEST1");

        // Add nextBlock size to Block size
        size_t nextBlock_size = nextBlock->header & BLOCK_SIZE_MASK;
        size_t overallSize = ptrBlock_size + nextBlock_size;

        // ptrBlock's header is header, combined size = block size, nextBlock's footer becomes footer

        // Fix size of header
        ptrBlock->header = (overallSize | 2); // 10 because previously is allocated

        void *nextBlock_afterAddr1 = ptrBlock_footer + nextBlock_size; // block after nextBlock (address)
        sf_block *nextBlock_after = (sf_block *) nextBlock_afterAddr1; // block after newBlock (block
        nextBlock_after->prev_footer = (overallSize | 2); // Footer same as header

        // Since next block is free, we assume that the PAL of the block after that is already 0

        // newBlock declarations
        newBlock_address = pointer;
        newBlock = (sf_block *) pointer;

    }

    // If nextBlock is ALLOCATED but prevBlock is FREE
    else if ( (nextBlockAllocated == 1) && (prevBlockAllocated != 1) ) {

        // Remove Prev Block LINKS:
        sf_block *nextFreeBlock = prevBlock->body.links.next;
        sf_block *prevFreeBlock = prevBlock->body.links.prev;
        nextFreeBlock->body.links.prev = prevBlock->body.links.prev; // REMOVE FROM FREE LIST
        prevFreeBlock->body.links.next = prevBlock->body.links.next;
        prevBlock->body.links.next = NULL;
        prevBlock->body.links.prev = NULL;

        // debug("TEST2");

        // Add prevBlock size to Block size
        size_t prevBlock_size = prevBlock->header & BLOCK_SIZE_MASK;
        size_t overallSize = ptrBlock_size + prevBlock_size;

        // Start from prevBlock header, end at ptrBlock footer

        // Fix size of header
        int prevBlock_prevAllocated = prevBlock->header & PREV_BLOCK_ALLOCATED; // If block before prevBlock is allocated
        prevBlock->header = (overallSize | prevBlock_prevAllocated);

        // Fix size of ptrBlock's footer
        nextBlock->prev_footer = (overallSize | prevBlock_prevAllocated);

        // next block should have pal = 0 (change its header and footer)
        // The block will always be an allocated block
        int nextBlock_prevAlloc = (int) nextBlock->header & PREV_BLOCK_ALLOCATED;
        if (nextBlock_prevAlloc == 2) nextBlock->header = (nextBlock->header) - 2;
        nextBlock->header = (nextBlock->header) - 2; // Take off two because you're making this block free (and PAL was previously 1)

        // newBlock declarations

        newBlock_address = prevBlock_address;
        newBlock = (sf_block *) prevBlock_address;

    }

    // Both nextBlock and prevBLOCK are ALLOCATED
    else if ( (nextBlockAllocated == 1) && (prevBlockAllocated == 1) ) {
        // No coalescing, just place into heap

        // Change header and footer to alloc = 1.

        // debug("TEST 3");

        int ptrBlock_alloc = (int) ptrBlock->header & THIS_BLOCK_ALLOCATED;
        if (ptrBlock_alloc == 1) ptrBlock->header = ptrBlock->header - 1; // taking out ALLOCATED bit.

        nextBlock->prev_footer = ptrBlock->header; // footer equivalent to header

        // Next block should have pal = 0. (change it's header and footer)
        // The next block will always be an allocated block, so no footer
        // int nextblock_allocated = nextBlock

        int nextBlock_prevAlloc = (int) nextBlock->header & PREV_BLOCK_ALLOCATED;
        if (nextBlock_prevAlloc == 2) nextBlock->header = (nextBlock->header) - 2;

        // NewBlock declarations
        newBlock_address = pointer;
        newBlock = (sf_block *) pointer;

    }

    else {

        // Both prevBlock and nextBlock are FREE! (COALESCE BOTH)

                // debug("TEST4");

        size_t prevBlock_size = prevBlock->header & BLOCK_SIZE_MASK;
        size_t nextBlock_size = nextBlock->header & BLOCK_SIZE_MASK;
        size_t overallSize = prevBlock_size + ptrBlock_size + nextBlock_size;

        // Remove from free list arrays:

        // Remove Prev Block LINKS:
        sf_block *nextFreeBlock = prevBlock->body.links.next;
        sf_block *prevFreeBlock = prevBlock->body.links.prev;
        nextFreeBlock->body.links.prev = prevBlock->body.links.prev; // REMOVE FROM FREE LIST
        prevFreeBlock->body.links.next = prevBlock->body.links.next;
        prevBlock->body.links.next = NULL;
        prevBlock->body.links.prev = NULL;

        // Remove Next Block LINKS:
        sf_block *nextFreeBlock_next = nextBlock->body.links.next;
        sf_block *prevFreeBlock_next = nextBlock->body.links.prev;
        nextFreeBlock_next->body.links.prev = nextBlock->body.links.prev; // REMOVE FROM FREE LIST
        prevFreeBlock_next->body.links.next = nextBlock->body.links.next;
        nextBlock->body.links.next = NULL;
        nextBlock->body.links.prev = NULL;

        // Start at prevBlock header, end at nextBlock footer


        // Fix size of nextBlock's footer
        void *nextBlock_afterAddr1 = ptrBlock_footer + nextBlock_size; // block after nextBlock (address)
        sf_block *nextBlock_after = (sf_block *) nextBlock_afterAddr1; // block after newBlock (block

        // save Previous Block's PAL

        int prevBlock_prevAllocated = prevBlock->header & PREV_BLOCK_ALLOCATED;
        prevBlock->header = (overallSize | prevBlock_prevAllocated); // previous is allocated
        nextBlock_after->prev_footer = (overallSize | prevBlock_prevAllocated); // Footer same as header

        // newBlock declarations

        newBlock_address = prevBlock_address;
        newBlock = (sf_block *) prevBlock_address;

        // Change PAL of after next block (both HEADER AND FOOTER)


    }

    // ------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------
    // FROM HERE: Determine size class, place at beginning of free list for that size class

    size_t newBlock_size = newBlock->header & BLOCK_SIZE_MASK;
    int listIndex;

    // Check if it's WILDERNESS BLOCK
    void *newEpilogue_address = sf_mem_end() - 8;
    if ( (newBlock_address + newBlock_size + 8) == newEpilogue_address) listIndex = 9; // IT IS THE WILDERNESS BLOCK
    else listIndex = findIndex(newBlock_size); // IT IS NOT THE WILDERNESS BLOCK

    // Add it to the appropriate linked list (AT BEGINNING)
    newBlock->body.links.next = sf_free_list_heads[listIndex].body.links.next; // newBlock's next is former 1st place
    (sf_free_list_heads[listIndex].body.links.next)->body.links.prev = newBlock; // Former 1st place's prev is newBlock
    newBlock->body.links.prev = &sf_free_list_heads[listIndex]; // newBlock's prev is sentinel node
    sf_free_list_heads[listIndex].body.links.next = newBlock; // sentintel node's next is newBlock

    return newBlock;
}

// Returns -1 if failure, 0 if success
int checkPowerofTwo(size_t number) {

    if (number == 0) return -1;
    // Else, traverse:

    while (number != 1) {

        if (number % 2 > 0) return -1;
        else number /= 2; // next iteration

    } // end of while loop
    return 0;
}

size_t calculateSizeAlign(size_t size, size_t align) {

    size += align;
    size += 64;
    // size += 8;
    return size;
}