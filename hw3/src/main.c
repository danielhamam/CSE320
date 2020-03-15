#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

// --------------------------------------------------
    // sf_show_block(sf_block *bp);

    sf_mem_init();
    sf_mem_grow();
    sf_show_blocks();
    // sf_show_free_list(int index);
    sf_show_free_lists();
    sf_show_heap();
    return 0;
// -------------------------------------------------


    sf_mem_init();

    double* ptr = sf_malloc(sizeof(double));

    *ptr = 320320320e-320;

    printf("%f\n", *ptr);

    sf_free(ptr);

    sf_mem_fini();

    return EXIT_SUCCESS;
}
