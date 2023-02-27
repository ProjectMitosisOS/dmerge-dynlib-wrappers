#include <dlfcn.h>
#include <cstddef>
#include "include/allocator.hh"
#include <cstdlib>
#include <sys/mman.h>

static void *(*real_malloc)(size_t) = NULL;

static void *(*real_realloc)(void *, size_t) = NULL;

static void *(*real_calloc)(size_t, size_t) = NULL; // not used

static void (*real_free)(void *) = NULL;

static void *(*real_reallocf)(void *, size_t) = NULL;

static void *(*real_valloc)(size_t) = NULL;

static void *(*real_aligned_alloc)(size_t, size_t) = NULL;

static void *(*real_mmap)(void *, size_t, int, int, int, off_t) = NULL;

static int (*real_munmap)(void *, size_t) = NULL;

static uint64_t BASE = 0x4ffff5a00000;
static uint64_t TOTAL_SZ = 1024 * 1024 * 1024;

/* Start of ZALLOC */
#define ZALLOC_MAX 1024
static void *zalloc_list[ZALLOC_MAX];
static size_t zalloc_cnt = 0;

static void *zalloc_internal(size_t size) {
    if (zalloc_cnt >= ZALLOC_MAX - 1) {
        fputs("alloc.so: Out of internal memory\n", stderr);
        return NULL;
    }
    /* Anonymous mapping ensures that pages are zero'd */
#if 0
    void *ptr = valloc(size);
    if (ptr == NULL) {
        perror("alloc.so: zalloc_internal valloc failed");
        return NULL;
    }
#else
    void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
    if (MAP_FAILED == ptr) {
        perror("alloc.so: zalloc_internal mmap failed");
        return NULL;
    }
#endif
    zalloc_list[zalloc_cnt++] = ptr; /* keep track for later calls to free */
    return ptr;
}
/* End of ZALLOC */

/**
 * Important: The `dlsym` would call `calloc` so that we
 * have to check whether there exists init process. Otherwise
 * it would cause a recursion to the end of the stack, and
 * incur a segfault.
 * Thanks to https://stackoverflow.com/a/10008252
 * */
static int alloc_init_pending = 0;

static void init(void) {
    alloc_init_pending = 1;

    real_malloc = AllocHelper::_malloc;
    real_realloc = AllocHelper::_realloc;
    real_calloc = AllocHelper::_calloc;
    real_free = AllocHelper::_free;
    real_reallocf = AllocHelper::_reallocf;
    real_valloc = AllocHelper::_valloc;
    real_aligned_alloc = AllocHelper::_aligned_alloc;
    real_mmap = (void *(*)(void *, size_t, int, int, int, off_t)) dlsym(
            RTLD_NEXT, "mmap");
    real_munmap = (int (*)(void *, size_t)) dlsym(RTLD_NEXT, "munmap");
    // Init for allocator
    char *end;
    BASE = strtoll(getenv("BASE_HEX"), &end, 16);
    TOTAL_SZ = strtoll(getenv("TOTAL_SZ_HEX"), &end, 16);
    AllocHelper::_init(BASE, TOTAL_SZ);
    alloc_init_pending = 0;
}

extern "C" {

void *calloc(size_t count, size_t size) {
    if (alloc_init_pending) {
//        fputs("[calloc] pending\n", stderr);
        return zalloc_internal(count * size);
    }
    if (!real_calloc) {
        init();
    }
    return real_calloc(count, size);
}

void *malloc(size_t size) {
    if (alloc_init_pending) {
//        fputs("[malloc] pending\n", stderr);
        return zalloc_internal(size);
    }
    if (!real_malloc) {
        init();
    }
    void *ptr = real_malloc(size);
    return ptr;
}

void *realloc(void *ptr, size_t size) {
    if (alloc_init_pending) {
//        fputs("[realloc] pending\n", stderr);
        return zalloc_internal(size);
    }
    if (!real_realloc) {
        init();
    }
    return real_realloc(ptr, size);
}

void free(void *ptr) {
    if (alloc_init_pending) {
        return;
    }
    if (!real_free) {
        init();
    }
    if (!ptr) return;
    real_free(ptr);
}

static inline void *my_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    void *
    (*fetch_mmap)(void *, size_t, int, int, int, off_t) = (void *(*)(void *, size_t, int, int, int, off_t)) dlsym(
            RTLD_NEXT, "mmap");
    return fetch_mmap(addr, length, prot, fd, flags, offset);
}

static inline int my_munmap(void *addr, size_t length) {
    int (*fetch_munmap)(void *, size_t) = (int (*)(void *, size_t)) dlsym(RTLD_NEXT, "munmap");
    return fetch_munmap(addr, length);
}
#if 0
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    if (!real_mmap) {
        init();
    }
//    real_mmap = (void *(*)(void *, size_t, int, int, int, off_t)) dlsym(
//            RTLD_NEXT, "mmap");
    return real_mmap(addr, length, prot, flags, fd, offset);
    if (fd == -1) { // for anon region mmap
        fputs("[mmap] malloc path\n", stderr);
        void *result = real_malloc(length);
        if (result == NULL) {
            errno = ENOMEM;
            return MAP_FAILED;
        }
        fputs("[mmap] malloc success\n", stderr);

        return result;
    } else {
        fputs("[mmap] real_mmap path\n", stderr);
        return real_mmap(addr, length, prot, flags, fd, offset);
    }
}

int munmap(void *addr, size_t length) {
    if (!real_munmap) {
        init();
    }
    return real_munmap(addr, length);
    int ret = 0;
    if (addr != NULL) {
        void *base = NULL;
        size_t size = 0;
        if (jemallctl("arenas/extent_hooks/base", &base, &size, &addr, sizeof(void *)) == 0) {
            fputs("[munmap] free path\n", stderr);
            real_free(addr);
        } else {
            fputs("[munmap] real_munmap path\n", stderr);
            return real_munmap(addr, length);
        }
    }
    return ret;
}
#endif

}
