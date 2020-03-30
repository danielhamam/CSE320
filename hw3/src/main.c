#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {


    sf_malloc(1600);
    char *x = sf_malloc(200);
    sf_memalign(300,512);
    sf_free(x);
    char *y = sf_malloc(2000);
    sf_realloc(y,1000);
    sf_free(y);
    char *z = sf_malloc(2033);
    sf_realloc(z, 200);
    sf_memalign(300, 1024);

    // void *y = sf_malloc(40);
    // sf_realloc(y, 325);
    // sf_memalign(160, 1024);
    // void *b = sf_malloc(250);
    // sf_free(b);
    // sf_malloc(52);
//
    // sf_memalign(300, 512);
    // sf_malloc(700);
    // sf_memalign(700,1024);

    sf_show_heap();
    sf_mem_fini(); // call at end of program to avoid valgrind errors

    return EXIT_SUCCESS;
}
