#include "hashmap.h"

#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "err.h"
#include "hash.h"
#include "memory.h"

#define A_HASHMAP_RESIZE_THRESHOLD_LOW  0.25
#define A_HASHMAP_RESIZE_THRESHOLD_HIGH 0.75
#define A_HASHMAP_RESIZE_FACTOR         2U

static a_Err_t a_Hashmap_Resize(a_Hashmap_t *const hashmap, const size_t capacity);
static double a_Hashmap_GetSizeFactor(const a_Hashmap_t *const hashmap);
static size_t a_Hashmap_GetIndex(a_Hashmap_t *const hashmap, const void *const key);
static void *a_Hashmap_GetEntry(const a_Hashmap_t *const hashmap, const void *const key);
static void *a_Hashmap_GetEntryValue(const a_Hashmap_t *const hashmap, const uint8_t *const entry);
static void *a_Hashmap_GetEntryNext(const a_Hashmap_t *const hashmap, const uint8_t *const entry);
static void a_Hashmap_SetEntryKey(const a_Hashmap_t *const hashmap, uint8_t *const entry, const void *const key);
static void a_Hashmap_SetEntryValue(const a_Hashmap_t *const hashmap, uint8_t *const entry, const void *const value);
static void a_Hashmap_SetEntryNext(const a_Hashmap_t *const hashmap, uint8_t *const entry, const void *const next);

a_Err_t a_Hashmap_Initialize(a_Hashmap_t *const hashmap, const size_t key_size, const size_t value_size)
{
    a_Err_t error = A_ERR_NONE;

    if (NULL == hashmap)
    {
        error = A_ERR_NULL;
    }
    else if ((0U == key_size) || (0U == value_size))
    {
        error = A_ERR_SIZE;
    }
    else
    {
        hashmap->key_size   = key_size;
        hashmap->value_size = value_size;
        hashmap->capacity   = 1U;
        hashmap->size       = 0U;
        hashmap->entries    = a_calloc(hashmap->capacity, sizeof(void *));

        if (NULL == hashmap->entries)
        {
            error = A_ERR_MEMORY;
        }
    }

    return error;
}

void a_Hashmap_Deinitialize(a_Hashmap_t *const hashmap)
{
    if (NULL != hashmap)
    {
        hashmap->capacity = 0U;
        hashmap->size     = 0U;

        for (size_t i = 0U; i < hashmap->capacity; i++)
        {
            void *entry = *(hashmap->entries + i);

            while (NULL != entry)
            {
                void *next = a_Hashmap_GetEntryNext(hashmap, entry);

                a_free(entry);
                entry = next;
            }
        }
    }
}

a_Err_t a_Hashmap_Insert(a_Hashmap_t *const hashmap, const void *const key, const void *const value)
{
    a_Err_t error = A_ERR_NULL;

    if ((NULL != hashmap) && (NULL != key) && (NULL != value))
    {
        error = A_ERR_NONE;

        void *entry = a_Hashmap_GetEntry(hashmap, key);

        if (NULL != entry)
        {
            a_Hashmap_SetEntryValue(hashmap, entry, value);
        }
        else
        {
            entry = a_calloc(1U, (hashmap->key_size + hashmap->value_size + sizeof(void *)));

            if (NULL == entry)
            {
                error = A_ERR_MEMORY;
            }
            else
            {
                const size_t index = a_Hashmap_GetIndex(hashmap, key);

                a_Hashmap_SetEntryKey(hashmap, entry, key);
                a_Hashmap_SetEntryValue(hashmap, entry, value);
                a_Hashmap_SetEntryNext(hashmap, entry, *(hashmap->entries + index));
                *(hashmap->entries + index) = entry;
                hashmap->size++;

                if (a_Hashmap_GetSizeFactor(hashmap) > A_HASHMAP_RESIZE_THRESHOLD_HIGH)
                {
                    error = a_Hashmap_Resize(hashmap, hashmap->capacity * A_HASHMAP_RESIZE_FACTOR);
                }
            }
        }
    }

    return error;
}

void *a_Hashmap_Get(const a_Hashmap_t *const hashmap, const void *const key)
{
    void *value = NULL;

    if ((NULL != hashmap) && (NULL != key))
    {
        void *entry = a_Hashmap_GetEntry(hashmap, key);

        if (NULL != entry)
        {
            value = a_Hashmap_GetEntryValue(hashmap, entry);
        }
    }

    return value;
}

a_Err_t a_Hashmap_Remove(a_Hashmap_t *const hashmap, const void *const key)
{
    a_Err_t error = A_ERR_NULL;

    if ((NULL != hashmap) && (NULL != key))
    {
        error = A_ERR_NONE;

        void *entry    = *(hashmap->entries + a_Hashmap_GetIndex(hashmap, key));
        void *previous = entry;

        while (NULL != entry)
        {
            if (0 == memcmp(entry, key, hashmap->key_size))
            {
                a_Hashmap_SetEntryNext(hashmap, previous, a_Hashmap_GetEntryNext(hashmap, entry));
                a_free(entry);
                hashmap->size--;

                if ((a_Hashmap_GetSizeFactor(hashmap) < A_HASHMAP_RESIZE_THRESHOLD_LOW) && (hashmap->size > 0U))
                {
                    error = a_Hashmap_Resize(hashmap, hashmap->capacity / A_HASHMAP_RESIZE_FACTOR);
                }
            }

            previous = entry;
            entry    = a_Hashmap_GetEntryNext(hashmap, entry);
        }
    }

    return error;
}

void a_Hashmap_ForEach(const a_Hashmap_t *const hashmap, void (*callback)(void *key, void *value, const void *const arg), const void *const arg)
{
    if ((NULL != hashmap) && (NULL != callback))
    {
        for (size_t i = 0U; i < hashmap->capacity; i++)
        {
            void *entry = *(hashmap->entries + i);

            while (NULL != entry)
            {
                callback(entry, a_Hashmap_GetEntryValue(hashmap, entry), arg);
                entry = a_Hashmap_GetEntryNext(hashmap, entry);
            }
        }
    }
}

static a_Err_t a_Hashmap_Resize(a_Hashmap_t *const hashmap, const size_t capacity)
{
    a_Err_t error       = A_ERR_MEMORY;
    void ** new_entries = a_calloc(capacity, sizeof(void *));

    if (NULL != new_entries)
    {
        for (size_t i = 0; i < hashmap->capacity; i++)
        {
            void *entry = *(hashmap->entries + i);

            while (NULL != entry)
            {
                void **new_entry = new_entries + a_Hashmap_GetIndex(hashmap, entry);
                void * next      = a_Hashmap_GetEntryNext(hashmap, entry);

                a_Hashmap_SetEntryNext(hashmap, entry, *new_entry);
                *new_entries = entry;
                entry        = next;
            }
        }

        free(hashmap->entries);
        hashmap->entries  = new_entries;
        hashmap->capacity = capacity;
        error             = A_ERR_NONE;
    }

    return error;
}

static double a_Hashmap_GetSizeFactor(const a_Hashmap_t *const hashmap)
{
    return (double)hashmap->size / (double)hashmap->capacity;
}

static size_t a_Hashmap_GetIndex(a_Hashmap_t *const hashmap, const void *const key)
{
    return (size_t)a_Hash_Value(key, hashmap->key_size) % hashmap->capacity;
}

static void *a_Hashmap_GetEntry(const a_Hashmap_t *const hashmap, const void *const key)
{
    void *entry = *(hashmap->entries + a_Hashmap_GetIndex(hashmap, key));

    while (NULL != entry)
    {
        if (0 == memcmp(entry, key, hashmap->key_size))
        {
            break;
        }

        entry = a_Hashmap_GetEntryNext(hashmap, entry);
    }

    return entry;
}

static void *a_Hashmap_GetEntryValue(const a_Hashmap_t *const hashmap, const uint8_t *const entry)
{
    return entry + hashmap->key_size;
}

static void *a_Hashmap_GetEntryNext(const a_Hashmap_t *const hashmap, const uint8_t *const entry)
{
    return entry + (hashmap->key_size + hashmap->value_size);
}

static void a_Hashmap_SetEntryKey(const a_Hashmap_t *const hashmap, uint8_t *const entry, const void *const key)
{
    memcpy(entry, key, hashmap->key_size);
}

static void a_Hashmap_SetEntryValue(const a_Hashmap_t *const hashmap, uint8_t *const entry, const void *const value)
{
    memcpy(a_Hashmap_GetEntryValue(hashmap, entry), value, hashmap->value_size);
}

static void a_Hashmap_SetEntryNext(const a_Hashmap_t *const hashmap, uint8_t *const entry, const void *const next)
{
    memcpy(a_Hashmap_GetEntryNext(hashmap, entry), &next, sizeof(void *));
}
