#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    // sf_malloc(64);
    void *x = sf_malloc(1);
    void *y = sf_malloc(65);
    void *z = sf_malloc(324);
    void *b = sf_malloc(512);
    void *c = sf_malloc(3850);

    sf_realloc(x, 64);
    sf_realloc(y, 32);
    sf_realloc(z, 128);

    sf_realloc(b, 256);
    sf_realloc(c, 2048);




    // sf_memalign(300, 512);
    // sf_malloc(sizeof(double) * 8);
    // sf_realloc(x, 350);

    // char *x = sf_malloc(200);
    // sf_free(x);
    // char *y = sf_malloc(2000);
    // sf_realloc(y, 1000);
    // sf_free(y);
    // char *z = sf_malloc(2033);
    // sf_realloc(z, 200);


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
