#ifndef AETHER_HASH_H
#define AETHER_HASH_H

#include <limits.h>
#include <stddef.h>

#define A_HASH_MAX ULONG_MAX

typedef unsigned long a_Hash_t;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

a_Hash_t a_Hash_Value(const void *const value, const size_t size);
a_Hash_t a_Hash_String(const char *const string);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AETHER_HASH_H */
