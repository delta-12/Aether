#ifndef AETHER_MEMORY_H
#define AETHER_MEMORY_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

void *a_malloc(const size_t size);
void *a_calloc(const size_t num, const size_t size);
void a_free(void *const ptr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AETHER_MEMORY_H */
