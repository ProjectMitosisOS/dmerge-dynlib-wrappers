#include <dlfcn.h>
#include <cstddef>
#include <cassert>
#include "include/allocator.hh"

static void *(*real_malloc)(size_t) = NULL;

static void *(*real_realloc)(void *, size_t) = NULL;

static void *(*real_calloc)(size_t, size_t) = NULL; // not used

static void (*real_free)(void *) = NULL;

static void *(*real_reallocf)(void *, size_t) = NULL;

static void *(*real_valloc)(size_t) = NULL;

static void *(*real_aligned_alloc)(size_t, size_t) = NULL;


static uint64_t BASE = 0x4ffff5a00000;
static uint64_t TOTAL_SZ = 1024 * 1024 * 1024;


static void init(void) {
#if 1
    real_malloc = (void *(*)(size_t)) dlsym(RTLD_NEXT, "malloc");
    real_calloc = (void *(*)(size_t, size_t)) dlsym(RTLD_NEXT, "calloc");
    real_realloc = (void *(*)(void *, size_t)) dlsym(RTLD_NEXT, "realloc");
    real_free = (void (*)(void *)) dlsym(RTLD_NEXT, "free");
    real_reallocf = (void *(*)(void *, size_t)) dlsym(RTLD_NEXT, "reallocf");
    real_valloc = (void *(*)(size_t)) dlsym(RTLD_NEXT, "valloc");
    real_aligned_alloc = (void *(*)(size_t, size_t)) dlsym(RTLD_NEXT, "aligned_alloc");
#else
    // Init for allocator
    AllocHelper::_init(BASE, TOTAL_SZ);
    real_malloc = AllocHelper::_malloc;
    real_realloc = AllocHelper::_realloc;
    real_free = AllocHelper::_free;
    real_reallocf = AllocHelper::_reallocf;
    real_valloc = AllocHelper::_valloc;
    real_aligned_alloc = AllocHelper::_aligned_alloc;
#endif
}

extern "C" {
//void *calloc(size_t count, size_t size) {
//    if (!real_calloc) init();
////    void *ptr = real_malloc(count * size);
////    memset(ptr, 0, count * size);
////    return ptr;
//    return real_calloc(count, size);
//}

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
void *
reallocf(void *ptr, size_t size) {
    if (!real_reallocf) init();
    assert(0);
    return real_reallocf(ptr, size);
}

void *
valloc(size_t size) {
    if (!real_valloc) init();
    assert(0);
    return real_valloc(size);
}

void *
aligned_alloc(size_t alignment, size_t size) {
    if (!real_aligned_alloc) init();
    assert(0);
    return real_aligned_alloc(alignment, size);
}
}
