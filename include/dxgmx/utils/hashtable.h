/**
 * Copyright 2022 Alexandru Olaru.
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

/* Initialized the hashtable by allocating the specified number of entries for 'ht'. */
bool hashtable_init(size_t entries, HashTable *ht);
/** 
 * Add a new entry to the hashtable.
 * @returns true if the the item has been added to the hashtable, or false if
 * the hashtable is full and needs enlaring: see hashtable_enlarge(). 
*/
bool hashtable_add(size_t key, const void *data, HashTable *ht);
/* Returns the value for the specified 'key'. */
void *hashtable_get(size_t key, HashTable *ht);
/* Remove the entry for the given 'key'. */
void hashtable_remove(size_t key, HashTable *ht);
void hashtable_destroy(HashTable *ht);
bool hashtable_enlarge(size_t size, HashTable *ht);

#endif //!_DXGMX_HASHTABLE_H
