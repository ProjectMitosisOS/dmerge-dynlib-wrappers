#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("main func addr:%p\n", main);
    printf("start malloc\n");
    char *pc1 = malloc(10);
    char *pc2 = malloc(10);
    printf("start free\n");
    free(pc1);
}
