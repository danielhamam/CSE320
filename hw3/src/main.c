#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    void *y = sf_malloc(40);
    sf_realloc(y, 325);
    sf_memalign(160, 1024);
    void *b = sf_malloc(250);
    sf_free(b);
    sf_malloc(52);
//
    // sf_memalign(100, 256);

    sf_show_heap();
    sf_mem_fini(); // call at end of program to avoid valgrind errors

    return EXIT_SUCCESS;
}
