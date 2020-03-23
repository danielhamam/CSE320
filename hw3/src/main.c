#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    // sf_mem_init();

    /* void *x = */ sf_malloc(8);
    sf_malloc(32);
    /* void *z = */ sf_malloc(1);
    // ---------------------
        sf_show_heap();

    // sf_free(y);
    sf_mem_fini();
    return 0;
    // ---------------------

    // *ptr = 320320320e-320;

    // printf("%f\n", *ptr);

    // sf_free(ptr);

    sf_mem_fini();

    return EXIT_SUCCESS;
}
