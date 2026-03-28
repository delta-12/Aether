#include "memory.h"

#ifdef AETHER_MEMORY_STATIC_ALLOCATOR
#include <stddef.h>
#include <stdint.h>
#else
#include <stdlib.h>
#endif /* AETHER_MEMORY_STATIC_ALLOCATOR */

#ifdef AETHER_MEMORY_STATIC_ALLOCATOR
#ifndef AETHER_MEMORY_STATIC_ALLOCATOR_SIZE
#define AETHER_MEMORY_STATIC_ALLOCATOR_SIZE 4096U
#endif /* AETHER_MEMORY_STATIC_ALLOCATOR_SIZE */

static uint8_t a_Memory_Pool[AETHER_MEMORY_STATIC_ALLOCATOR_SIZE];
#endif /* AETHER_MEMORY_STATIC_ALLOCATOR */

void *a_malloc(const size_t size)
{
#ifdef AETHER_MEMORY_STATIC_ALLOCATOR
    /* TODO */
    return NULL;
#else
    return malloc(size);
#endif /* AETHER_MEMORY_STATIC_ALLOCATOR */
}

void *a_calloc(const size_t num, const size_t size)
{
#ifdef AETHER_MEMORY_STATIC_ALLOCATOR
    /* TODO */
    return NULL;
#else
    return calloc(num, size);
#endif /* AETHER_MEMORY_STATIC_ALLOCATOR */
}

void *a_realloc(void *const ptr, const size_t size)
{
#ifdef AETHER_MEMORY_STATIC_ALLOCATOR
    /* TODO */
    return NULL;
#else
    return realloc(ptr, size);
#endif /* AETHER_MEMORY_STATIC_ALLOCATOR */
}

void a_free(void *const ptr)
{
#ifdef AETHER_MEMORY_STATIC_ALLOCATOR
    /* TODO */
#else
    free(ptr);
#endif /* AETHER_MEMORY_STATIC_ALLOCATOR */
}
