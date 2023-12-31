#include "jemalloc/jemalloc.h"
//#include "../deps/jemalloc/include/jemalloc/jemalloc.h"
#include <cstring>
#include <cstdint>
#include <mutex>
#include <functional>
#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <dlfcn.h>

#define unlikely(x) __builtin_expect(!!(x), 0)
#define likely(x) __builtin_expect(!!(x), 1)


using ptr_t = void *;


class Allocator {
public:
    explicit Allocator(unsigned id) : id(id) {
    }

    inline ptr_t alloc(uint32_t size, int flag = 0) {
        auto ptr = jemallocx(size, id | flag);
        return ptr;
    }

    inline void dealloc(ptr_t ptr) { this->free(ptr); }

    inline ptr_t realloc(ptr_t ptr, int n) {
        if (n == 0) {
            this->free(ptr);
            return NULL;
        }

        ptr_t new_ptr = this->alloc(n);

        if (ptr != NULL) {
            memcpy(new_ptr, ptr, n);
            free(ptr);
        }

        return new_ptr;
    }

    inline ptr_t calloc(size_t count, size_t size) {
        ptr_t res = this->alloc(count * size);
        memset(res, 0, count * size);
        return res;
    }

    inline void free(ptr_t ptr) {
        jedallocx(ptr, id);
    }

    inline ptr_t reallocf(ptr_t ptr, size_t size) {
        return jerealloc(ptr, size);
    }

    inline ptr_t valloc(size_t size) {
        return this->alloc(size);
    }

    inline ptr_t aligned_alloc(size_t alignment, size_t size) {
        return jealigned_alloc(alignment, size);
    }

private:
    unsigned id;
};

template<int NAME = 0>
class AllocatorMaster {
public:
    static void init(char *mem, uint64_t mem_size) {
        std::lock_guard<std::mutex> guard(lock);
        if (total_managed_mem() != 0) {
            std::cout << "AllocatorMaster<" << NAME << "> inited multiple times" << std::endl;
            return;
        }

        start_addr = mem;
        end_addr = start_addr + mem_size;
        heap_top = start_addr;
    }

    static Allocator *get_thread_allocator() {
        if (likely(thread_allocator))
            return thread_allocator;
        return thread_allocator = get_allocator();
    }

    static Allocator *get_allocator() {

        /**
         * First we make sanity check on whether AllocatorMaster is initialized
         */
        {
            std::lock_guard<std::mutex> guard(lock);
            if (total_managed_mem() == 0) // not initialized yet
                return nullptr;
        }

        unsigned arena_id, cache_id;
        size_t sz = sizeof(unsigned);

        extent_hooks_t *new_hooks = &hooks;
        jemallctl("arenas.create", (void *) (&arena_id), &sz,
                  (void *) (&new_hooks), sizeof(extent_hooks_t * ));
        jemallctl("tcache.create", (void *) (&cache_id), &sz, nullptr, 0);
        return new Allocator(MALLOCX_ARENA(arena_id) | MALLOCX_TCACHE(cache_id));
        //return new Allocator(MALLOCX_ARENA(arena_id));
    }

    static uint64_t total_managed_mem() {
        return end_addr - start_addr;
    }

    static bool within_range(ptr_t p) {
        char *c = static_cast<char *>(p);
        auto ret = c >= start_addr && c < end_addr;
        return ret;
    }

public:
    static char *start_addr;
    static char *end_addr;
private:
    static char *heap_top;
    static std::mutex lock;

    static extent_hooks_t hooks;

    static thread_local Allocator
            *
            thread_allocator;

private:
    /**
     * Hooks to different jemalloc callbacks.
     */
    static void *extent_alloc_hook(extent_hooks_t *extent_hooks, void *new_addr, size_t size,
                                   size_t alignment, bool *zero, bool *commit, unsigned arena_ind) {
        std::lock_guard<std::mutex> guard(lock);
        char *ret = (char *) heap_top;

        // align the return address
        if ((uintptr_t) ret % alignment != 0)
            ret = ret + (alignment - (uintptr_t) ret % alignment);
//        std::cout << size << "," << alignment << std::endl;
        if ((char *) ret + size >= (char *) end_addr) {
            std::cout << "exceed heap size. Want sz:" << size << std::endl;
            return nullptr;
        }

        heap_top = ret + size;
        if (*zero) // extent should be zeroed
            memset(ret, 0, size);
        return ret;
    }

    static bool extent_dalloc_hook(extent_hooks_t *extent_hooks, void *addr, size_t size,
                                   bool committed, unsigned arena_ind) {
        return true; // opt out
    }

    static void extent_destroy_hook(extent_hooks_t *extent_hooks, void *addr, size_t size,
                                    bool committed, unsigned arena_ind) {
        return;
    }

    static bool extent_commit_hook(extent_hooks_t *extent_hooks, void *addr, size_t size,
                                   size_t offset, size_t length, unsigned arena_ind) {
        return false; // commit should always succeed
    }

    static bool extent_decommit_hook(extent_hooks_t *extent_hooks, void *addr, size_t size,
                                     size_t offset, size_t length, unsigned arena_ind) {
        return false; // decommit should always succeed
    }

    static bool extent_purge_lazy_hook(extent_hooks_t *extent_hooks, void *addr, size_t size,
                                       size_t offset, size_t length, unsigned arena_ind) {
        return true; // opt out
    }

    static bool extent_purge_forced_hook(extent_hooks_t *extent_hooks, void *addr, size_t size,
                                         size_t offset, size_t length, unsigned arena_ind) {
        return true; // opt out
    }

    static bool extent_split_hook(extent_hooks_t *extent_hooks, void *addr, size_t size,
                                  size_t size_a, size_t size_b, bool committed, unsigned arena_ind) {
        return false; // split should always succeed
    }

    static bool extent_merge_hook(extent_hooks_t *extent_hooks, void *addr_a, size_t size_a,
                                  void *addr_b, size_t size_b, bool committed, unsigned arena_ind) {
        return false; // merge should always succeed
    }
};

template<int N> char *AllocatorMaster<N>::start_addr = nullptr;
template<int N> char *AllocatorMaster<N>::end_addr = nullptr;
template<int N> char *AllocatorMaster<N>::heap_top = nullptr;
template<int N> std::mutex AllocatorMaster<N>::lock;

template<int N>
thread_local Allocator
        *
        AllocatorMaster<N>::thread_allocator = nullptr;

template<int N>
extent_hooks_t AllocatorMaster<N>::hooks = {
        AllocatorMaster<N>::extent_alloc_hook,
        AllocatorMaster<N>::extent_dalloc_hook,
        AllocatorMaster<N>::extent_destroy_hook,
        AllocatorMaster<N>::extent_commit_hook,
        AllocatorMaster<N>::extent_decommit_hook,
        AllocatorMaster<N>::extent_purge_lazy_hook,
        AllocatorMaster<N>::extent_purge_forced_hook,
        AllocatorMaster<N>::extent_split_hook,
        AllocatorMaster<N>::extent_merge_hook
};


class AllocHelper {
    using Alloc = AllocatorMaster<73>;
public:
    static void *_malloc(size_t size) {
        return (void *) Alloc::get_thread_allocator()->alloc(size);
    }

    static void *_realloc(ptr_t ptr, size_t n) {
        return (void *) Alloc::get_thread_allocator()->realloc(ptr, n);
    }

    static void *_calloc(size_t count, size_t size) {
        return (void *) Alloc::get_thread_allocator()->calloc(count, size);
    }

    static void _free(void *ptr) {
        Alloc::get_thread_allocator()->dealloc(ptr);
    }

    static void *_reallocf(void *ptr, size_t size) {
        return Alloc::get_thread_allocator()->reallocf(ptr, size);
    }

    static void *_valloc(size_t size) {
        return Alloc::get_thread_allocator()->valloc(size);
    }

    static void *_aligned_alloc(size_t alignment, size_t size) {
        return Alloc::get_thread_allocator()->aligned_alloc(alignment, size);
    }

    static void _init(uint64_t base_addr, uint64_t mem_sz) {
        auto ptr = mmap((void *) base_addr, mem_sz,
                        PROT_READ | PROT_WRITE | PROT_EXEC,
                        MAP_PRIVATE | MAP_ANON, -1, 0);
        Alloc::init((char *) ptr, mem_sz);
        for (int i = 0; i < mem_sz; ++i) {
            *(char *) (base_addr + i) = '\0';
        }
    }
};
