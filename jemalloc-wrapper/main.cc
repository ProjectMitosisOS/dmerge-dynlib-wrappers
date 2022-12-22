#include <stdio.h>
#include <stdlib.h>
#include <jemalloc/jemalloc.h>

int main() {
    printf("main func addr:%p\n", main);

    char *pc1 = (char *) malloc(10);
    printf("[main] ptr 1: %p\n", pc1);
    char *pc2 = (char *) malloc(10);
    printf("[main] ptr 2: %p\n", pc2);

    free(pc1);
    free(pc2);
}
