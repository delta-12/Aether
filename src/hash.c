#include "hash.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define A_HASH_SEED 5381U /* DJB2 seed */

a_Hash_t a_Hash_Value(const void *const value, const size_t size)
{
    /* DJB2 Hash Function */

    a_Hash_t hash = A_HASH_SEED;

    if (NULL != value)
    {
        for (size_t i = 0U; i < size; i++)
        {
            hash = ((hash << 5U) + hash) + *((const uint8_t *const)value + i);
        }
    }

    return hash;
}

a_Hash_t a_Hash_String(const char *const string)
{
    return a_Hash_Value(string, strlen(string));
}
