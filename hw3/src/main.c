#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    // sf_mem_init();

    void *x = sf_malloc(3 * PAGE_SZ - ((1 << 6) - sizeof(sf_header)) - 64 - 2*sizeof(sf_header));

    sf_free(x);
          sf_show_heap();
    return 0;



    sf_show_heap();
    sf_mem_fini(); // call at end of program to avoid valgrind errors
    return 0;
    // ---------------------

    // *ptr = 320320320e-320;

    // printf("%f\n", *ptr);

    // sf_free(ptr);

    sf_mem_fini();

    return EXIT_SUCCESS;
}
