#define _GNU_SOURCE

#include <dlfcn.h>
#include <cstddef>
#include <cstdio>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "include/allocator.hh"

//using Alloc = AllocatorMaster<73>;

static void *(*real_malloc)(size_t) = NULL;

static void (*real_free)(void *) = NULL;

using Alloc = AllocatorMaster<73>;

static void __attribute__((constructor)) init(void) {
    real_malloc = (void *(*)(size_t)) dlsym(RTLD_NEXT, "malloc");
    real_free = (void (*)(void *)) dlsym(RTLD_NEXT, "free");

    // Init for allocator
    {
        uint64_t base_addr = 0x4ffff5a00000;
        uint64_t mem_sz = 1024 * 1024 * 512;
        auto ptr = mmap((void *) base_addr, mem_sz,
                        PROT_READ | PROT_WRITE | PROT_EXEC,
                        MAP_PRIVATE | MAP_ANON, -1, 0);
        Alloc::init((char *) ptr, mem_sz);
        for (int i = 0; i < mem_sz; ++i) {
            *(char *) (base_addr + i) = '\0';
        }
    }
}


static inline void *_malloc(size_t size) {
    return (void *) Alloc::get_thread_allocator()->alloc(size);
}

static inline void _free(void *ptr) {
    Alloc::get_thread_allocator()->dealloc(ptr);
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
