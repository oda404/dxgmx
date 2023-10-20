/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/errno.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/string.h>
#include <dxgmx/utils/hashtable.h>

#define INITIAL_HASHTABLE_SLOTS 8
#define INITIAL_ENLARGE_THRESHOLD 4

static int hashtable_maybe_enlarge(HashTable* ht)
{
    if (ht->biggest_slot_entry_count < ht->slot_entry_count_threshold)
        return 0;

    HashTableSlot* tmp =
        krealloc(ht->slots, (ht->slot_capacity * 2) * sizeof(HashTableSlot));
    if (!tmp)
        return -ENOMEM;

    ht->slots = tmp;
    memset(
        ht->slots + ht->slot_capacity,
        0,
        ht->slot_capacity * sizeof(HashTableSlot));

    ht->slot_capacity *= 2;
    ht->slot_entry_count_threshold *= 2;
    return 0;
}

static ERR_OR(size_t)
    hashtable_key_to_slot_idx(hashtable_key_t key, HashTable* ht)
{
#define STEP 4
    // Is this true for all hash funcs? idk
    ASSERT(ht->hashfunc->digest_size % STEP == 0);

    u32 digest[ht->hashfunc->digest_size / STEP];
    int st = ht->hashfunc->chew(&key, sizeof(key), digest);
    if (st < 0)
        return ERR(size_t, st);

    u32 hashidx = 0;
    for (size_t i = 0; i < ht->hashfunc->digest_size / STEP; ++i)
        hashidx ^= digest[i];

    return VALUE(size_t, hashidx % ht->slot_capacity);
#undef STEP
}

static int hashtable_add_value_to_slot(
    hashtable_key_t key, hashtable_val_t value, HashTableSlot* slot)
{
    for (size_t i = 0; i < slot->entry_count; ++i)
    {
        HashTableEntry* entry = &slot->entries[i];
        if (entry->key == key)
        {
            entry->value = value;
            return 0;
        }
    }

    HashTableEntry* tmp = krealloc(
        slot->entries, (slot->entry_count + 1) * sizeof(HashTableEntry));
    if (!tmp)
        return -ENOMEM;

    slot->entries = tmp;
    ++slot->entry_count;
    slot->entries[slot->entry_count - 1].key = key;
    slot->entries[slot->entry_count - 1].value = value;
    return 0;
}

static HashTableEntry*
hashtable_find_entry(hashtable_key_t key, const HashTableSlot* slot)
{
    for (size_t i = 0; i < slot->entry_count; ++i)
    {
        HashTableEntry* entry = &slot->entries[i];
        if (entry->key == key)
            return entry;
    }
    return NULL;
}

int hashtable_init(HashTable* ht)
{
    memset(ht, 0, sizeof(HashTable));

    ht->hashfunc = hashing_best_func_by_speed();
    if (!ht->hashfunc)
        return -ENODEV;

    ht->slots = kcalloc(sizeof(HashTableSlot) * INITIAL_HASHTABLE_SLOTS);
    if (!ht->slots)
        return -ENOMEM;

    hashing_increase_refcount(ht->hashfunc);
    ht->slot_capacity = INITIAL_HASHTABLE_SLOTS;
    ht->slot_entry_count_threshold = INITIAL_ENLARGE_THRESHOLD;
    return 0;
}

int hashtable_insert(hashtable_key_t key, hashtable_val_t value, HashTable* ht)
{
    int st = hashtable_maybe_enlarge(ht);
    if (st < 0)
        return st;

    ERR_OR(size_t) hash_res = hashtable_key_to_slot_idx(key, ht);
    if (hash_res.error)
        return hash_res.error;

    return hashtable_add_value_to_slot(key, value, &ht->slots[hash_res.value]);
}

ERR_OR(hashtable_val_t) hashtable_find(hashtable_key_t key, HashTable* ht)
{
    ERR_OR(size_t) hash_res = hashtable_key_to_slot_idx(key, ht);
    if (hash_res.error)
        return ERR(hashtable_val_t, hash_res.error);

    HashTableEntry* entry =
        hashtable_find_entry(key, &ht->slots[hash_res.value]);
    if (!entry)
        return ERR(hashtable_val_t, -ENOENT);

    return VALUE(hashtable_val_t, entry->value);
}

int hashtable_remove(hashtable_key_t key, HashTable* ht)
{
    ERR_OR(size_t) hash_res = hashtable_key_to_slot_idx(key, ht);
    if (hash_res.error)
        return hash_res.error;

    HashTableSlot* slot = &ht->slots[hash_res.value];
    HashTableEntry* entry = hashtable_find_entry(key, slot);
    if (!entry)
        return -ENOENT;

    for (HashTableEntry* e = entry; e < slot->entries + slot->entry_count - 1;
         ++e)
        *e = *(e + 1);

    if (slot->entry_count == 1)
    {
        kfree(slot->entries);
        slot->entries = NULL;
    }
    else
    {
        slot->entries = krealloc(
            slot->entries, (slot->entry_count - 1) * sizeof(HashTableEntry));
        ASSERT(slot->entries);
    }

    --slot->entry_count;
    return 0;
}

void hashtable_destroy(HashTable* ht)
{
    FOR_EACH_ELEM_IN_DARR (ht->slots, ht->slot_capacity, slot)
    {
        if (!slot->entries)
            continue;

        kfree(slot->entries);
        slot->entries = NULL;
        slot->entry_count = 0;
    }

    if (ht->slots)
        kfree(ht->slots);

    hashing_decrease_refcount(ht->hashfunc);
    memset(ht, 0, sizeof(HashTable));
}
