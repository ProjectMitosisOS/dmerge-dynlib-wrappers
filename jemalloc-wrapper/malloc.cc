#include <dlfcn.h>
#include <cstddef>
#include <cstdio>

#include "include/allocator.hh"

static void *(*real_malloc)(size_t) = NULL;

static void (*real_free)(void *) = NULL;


static void init(void) {
#if 1
    AllocHelper::_init(0x4ffff5a00000, 1024 * 1024 * 1024 * 1);
    real_malloc = (void *(*)(size_t)) dlsym(RTLD_NEXT, "malloc");
    real_free = (void (*)(void *)) dlsym(RTLD_NEXT, "free");
#else
    // Init for allocator
    real_malloc = AllocHelper::_malloc;
    real_free = AllocHelper::_free;
#endif
}

void *malloc(size_t size) {
    if (!real_malloc) init();
    void *ptr = real_malloc(size);
    fprintf(stderr, "malloc(%zd) = %p\n", size, ptr);
    return ptr;
}

void free(void *ptr) {
    if (!real_free) init();
    if (!ptr) return;
    real_free(ptr);
    fprintf(stderr, "free(%p)\n", ptr);
}