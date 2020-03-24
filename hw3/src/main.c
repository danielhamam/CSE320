#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    // sf_mem_init();
    void *x = sf_malloc(sizeof(double) * 8);
    sf_realloc(x, sizeof(int));

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
