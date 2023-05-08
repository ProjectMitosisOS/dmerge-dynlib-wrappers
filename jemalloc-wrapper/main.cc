#include <cstdio>
#include <cstdlib>
#include <sys/mman.h>

int main() {
    char* preload_path = "libmalloc_wrapper.so";
    printf("main func addr:%p\n", main);

    char *pc1 = (char *) malloc(10);
    printf("[main] ptr 1: %p\n", pc1);
    setenv("LD_PRELOAD", preload_path, 1);
    char *pc2 = (char *) malloc(10);
    printf("[main] ptr 2: %p\n", pc2);
    unsetenv("LD_PRELOAD");
    char *calloc_ptr = (char *) calloc(512,10);
    printf("[main] calloc_ptr  %p\n", calloc_ptr);

    void* reallocp1 = realloc(pc1, 20);
    printf("[main] realloc  %p\n", reallocp1);


    free(calloc_ptr);
    free(pc1);
    free(pc2);
    void *ptr = mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
    printf("[main] mmap  %p\n", ptr);
}
