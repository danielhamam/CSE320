#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    // sf_mem_init();

    void *x = sf_malloc(8);
    void *abc = sf_malloc(32);
    void *z = sf_malloc(1);
    void *test1 = sf_malloc(900);
    void *test2 = sf_malloc(4000);
    // ---------------------

    sf_free(z);
    sf_free(x);
    sf_free(abc);
    sf_free(test1);
    sf_free(test2);

    sf_malloc(57);

            sf_show_heap();
    sf_mem_fini();
    return 0;
    // ---------------------

    // *ptr = 320320320e-320;

    // printf("%f\n", *ptr);

    // sf_free(ptr);

    sf_mem_fini();

    return EXIT_SUCCESS;
}
