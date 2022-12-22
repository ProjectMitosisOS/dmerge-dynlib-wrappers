#include <dlfcn.h>
#include <cstddef>
#include <cstdio>

#include "include/allocator.hh"

//using Alloc = AllocatorMaster<73>;

static void *(*real_malloc)(size_t) = NULL;

static void (*real_free)(void *) = NULL;


static void __attribute__((constructor)) init(void) {
//    real_malloc = (void *(*)(size_t)) dlsym(RTLD_NEXT, "malloc");
//    real_free = (void (*)(void *)) dlsym(RTLD_NEXT, "free");

    // Init for allocator
    AllocHelper::_init(0x4ffff5a00000, 1024 * 1024 * 1024 * 1);
//    AllocHelper::_malloc(32);
    real_malloc = AllocHelper::_malloc;
    real_free = AllocHelper::_free;
}


extern "C"
void *malloc(size_t len) {
    static __thread int no_hook = 0;
    if (!real_malloc) return 0;

    if (no_hook) {
        return (*real_malloc)(len);
    }

    void *caller = (void *) (long) __builtin_return_address(0);
    no_hook = 1;
    printf("malloc with len [%zu] from caller[%p]\n", len, caller); //printf call malloc internally
    no_hook = 0;

    void *ret = (*real_malloc)(len);

    return ret;
}

extern "C"
void free(void *ptr) {
    if (!real_free) return;
    void *caller = (void *) (long) __builtin_return_address(0);
    printf("free ptr %p from caller[%p]\n", ptr, caller);
    (*real_free)(ptr);
}
