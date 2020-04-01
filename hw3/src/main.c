#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    sf_show_heap();
    sf_mem_fini(); // call at end of program to avoid valgrind errors

    return EXIT_SUCCESS;
}
