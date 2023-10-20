/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_UTILS_HASHTABLE_H
#define _DXGMX_UTILS_HASHTABLE_H

#include <dxgmx/crypto/hashing.h>
#include <dxgmx/err_or.h>
#include <dxgmx/types.h>

typedef u64 hashtable_key_t;
typedef u64 hashtable_val_t;
DEFINE_ERR_OR(hashtable_val_t);

typedef struct HashTableEntry
{
    hashtable_key_t key;
    hashtable_val_t value;
} HashTableEntry;

typedef struct HashTableSlot
{
    HashTableEntry* entries;
    size_t entry_count;
} HashTableSlot;

typedef struct HashTable
{
    HashTableSlot* slots;
    size_t slot_capacity;

    size_t biggest_slot_entry_count;
    size_t slot_entry_count_threshold;

    HashingFunction* hashfunc;
} HashTable;

int hashtable_init(HashTable* ht);
int hashtable_insert(hashtable_key_t key, hashtable_val_t value, HashTable* ht);
ERR_OR(hashtable_val_t) hashtable_find(hashtable_key_t key, HashTable* ht);
int hashtable_remove(hashtable_key_t key, HashTable* ht);
void hashtable_destroy(HashTable* ht);

#endif //!_DXGMX_UTILS_HASHTABLE_H
