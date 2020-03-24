#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    // sf_mem_init();

    // sf_malloc(200);
    // /* void *v = */ sf_malloc(300);
    // sf_malloc(200);
    //  void *x =  sf_malloc(500);
    // sf_malloc(200);
    // /* void *z = */ sf_malloc(700);

    // sf_free(u);
    // sf_free(w);
    // sf_free(y);
    sf_malloc(3 * PAGE_SZ - ((1 << 6) - sizeof(sf_header)) - 64 - 2*sizeof(sf_header));
      sf_show_heap();
    return 0;



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
