#include <cstdio>
#include <cstdlib>

int main() {
    printf("main func addr:%p\n", main);

    char *pc1 = (char *) malloc(10);
    printf("[main] ptr 1: %p\n", pc1);
    char *pc2 = (char *) malloc(10);
    printf("[main] ptr 2: %p\n", pc2);
    char *calloc_ptr = (char *) calloc(512,10);
    printf("[main] calloc_ptr  %p\n", calloc_ptr);

    void* reallocp1 = realloc(pc1, 20);
    printf("[main] realloc  %p\n", reallocp1);


    free(calloc_ptr);
    free(pc1);
    free(pc2);
}
