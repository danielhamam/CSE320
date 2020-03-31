#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {


    // sf_malloc(1600);
    // char *x = sf_malloc(200);
    // sf_memalign(300,512);
    // sf_free(x);
    // char *y = sf_malloc(2000);
    // sf_realloc(y,1000);
    // sf_free(y);
    // char *z = sf_malloc(2033);
    // sf_realloc(z, 200);
    // sf_memalign(300, 1024);

    sf_memalign(730,1024);

    sf_show_heap();
    sf_mem_fini(); // call at end of program to avoid valgrind errors

    return EXIT_SUCCESS;
}
