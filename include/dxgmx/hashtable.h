/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_HASHTABLE_H
#define _DXGMX_HASHTABLE_H

#include<dxgmx/types.h>

typedef struct
S_HashTableEntry
{
    bool used;
    size_t key;
    const void *value;
} HashTableEntry;

typedef struct 
S_HashTable
{
    HashTableEntry *entries;
    size_t entries_max;
} HashTable;

bool hashtable_init_sized(size_t entries, HashTable *ht);
bool hashtable_init(HashTable *ht);
void hashtable_add(size_t key, const void *data, HashTable *ht);
void *hashtable_get(size_t key, HashTable *ht);
void hashtable_destroy(HashTable *ht);

#endif //!_DXGMX_HASHTABLE_H
