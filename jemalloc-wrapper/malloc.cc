#include <dlfcn.h>
#include <cstddef>
#include <cassert>
#include "include/allocator.hh"

static void *(*real_malloc)(size_t) = NULL;

static void *(*real_realloc)(void *, size_t) = NULL;

static void *(*real_calloc)(size_t, size_t) = NULL; // not used

static void (*real_free)(void *) = NULL;

static uint64_t BASE = 0x4ffff5a00000;
static uint64_t TOTAL_SZ = 1024 * 1024 * 1024;


static void init(void) {
#if 0
    real_malloc = (void *(*)(size_t)) dlsym(RTLD_NEXT, "malloc");
    real_realloc = (void *(*)(void *, size_t)) dlsym(RTLD_NEXT, "realloc");
    real_free = (void (*)(void *)) dlsym(RTLD_NEXT, "free");
#else
    // Init for allocator
    AllocHelper::_init(BASE, TOTAL_SZ);
    real_malloc = AllocHelper::_malloc;
    real_free = AllocHelper::_free;
    real_realloc = AllocHelper::_realloc;
#endif
}

extern "C" {
void *malloc(size_t size) {
    if (!real_malloc) init();
    void *ptr = real_malloc(size);
//    fprintf(stderr, "malloc(%zd) = %p, and remain sz:%lld\n", size, ptr, TOTAL_SZ - ((uint64_t) ptr - BASE));
    return ptr;
}

void *realloc(void *ptr, size_t size) {
    if (!real_realloc) init();
    return real_realloc(ptr, size);
}

void free(void *ptr) {
    if (!real_free) init();
    if (!ptr) return;
    real_free(ptr);
}
}
