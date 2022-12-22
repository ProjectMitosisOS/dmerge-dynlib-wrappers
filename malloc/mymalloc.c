#define _GNU_SOURCE

#include <dlfcn.h>
#include <stddef.h>
#include <stdio.h>

static void *(*real_malloc)(size_t) = NULL;

static void (*real_free)(void *) = NULL;

static void __attribute__((constructor)) init(void) {
    real_malloc = (void *(*)(size_t)) dlsym(RTLD_NEXT, "malloc");
    real_free = (void (*)(void *)) dlsym(RTLD_NEXT, "free");
}

void *malloc(size_t len) {
    static __thread int no_hook = 0;

    if (no_hook) {
        return (*real_malloc)(len);
    }

    void *caller = (void *) (long) __builtin_return_address(0);
    no_hook = 1;
    printf("malloc call %zu from %p\n", len, caller); //printf call malloc internally
    no_hook = 0;

    void *ret = (*real_malloc)(len);
    return ret;
}

void free(void *ptr) {
    void *caller = (void *) (long) __builtin_return_address(0);
    printf("free call %p from %p\n", ptr, caller);
    (*real_free)(ptr);
}

