#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    // sf_mem_init();

    double* ptr = sf_malloc(8);
    sf_free(ptr);
    // ---------------------
    sf_show_heap();
    return 0;
    // ---------------------

    *ptr = 320320320e-320;

    printf("%f\n", *ptr);

    sf_free(ptr);

    sf_mem_fini();

    return EXIT_SUCCESS;
}
