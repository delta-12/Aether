#ifndef AETHER_MEMORY_H
#define AETHER_MEMORY_H

#include <stddef.h>

void *a_malloc(const size_t size);
void *a_calloc(const size_t num, const size_t size);
void a_free(const void *const ptr);

#endif /* AETHER_MEMORY_H */
