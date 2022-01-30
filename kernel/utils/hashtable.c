/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/utils/hashtable.h>
#include<dxgmx/crypto/sha1.h>
#include<dxgmx/stdlib.h>
#include<dxgmx/string.h>
#include<dxgmx/kmalloc.h>
#include<dxgmx/todo.h>

bool hashtable_init(size_t entries, HashTable *ht)
{
    if(!entries || !ht)
        return false;

    ht->entries = kmalloc(entries * sizeof(HashTableEntry));
    if(!ht->entries)
        return false;
    memset(ht->entries, 0, entries * sizeof(HashTableEntry));

    ht->entries_max = entries;

    return true;
}

bool hashtable_add(size_t key, const void *data, HashTable *table)
{
    // check if table has entries
    if(!table->entries_max)
        return false;
    
    SHA1Digest digest = sha1_hash_buf((void*)&key, sizeof(key));

    size_t idx = digest.h0 ^ digest.h1 ^ digest.h2 ^ digest.h3 ^ ~digest.h4;
    idx %= table->entries_max;

    bool found = false;
    while(idx < table->entries_max)
    {
        if(!table->entries[idx].used)
        {
            found = true;
            break;
        }
        ++idx;
    }

    if(!found)
        return false; // if we got here the hash table is full.

    HashTableEntry *e = &table->entries[idx];
    e->key = key;
    e->value = data;
    e->used = true;

    return true;
}

void hashtable_remove(size_t key, HashTable *ht)
{
    SHA1Digest digest = sha1_hash_buf((void *)&key, sizeof(key));

    size_t idx = digest.h0 ^ digest.h1 ^ digest.h2 ^ digest.h3 ^ ~digest.h4;
    idx %= ht->entries_max;

    while(idx < ht->entries_max)
    {
        HashTableEntry *entry = &ht->entries[idx++];
        if(entry->key == key)
        {
            memset(entry, 0, sizeof(HashTableEntry));
            break;
        }
    }
}

void *hashtable_get(size_t key, HashTable *table)
{
    SHA1Digest digest = sha1_hash_buf((void *)&key, sizeof(key));

    size_t idx = digest.h0 ^ digest.h1 ^ digest.h2 ^ digest.h3 ^ ~digest.h4;
    idx %= table->entries_max;

    while(idx < table->entries_max)
    {
        HashTableEntry *entry = &table->entries[idx++];
        if(entry->key == key)
            return (void *)entry->value;
    }
    
    return NULL;
}

void hashtable_destroy(HashTable *ht)
{
    kfree(ht->entries);
}

bool hashtable_enlarge(size_t size, HashTable *ht)
{
    if(!size || !ht)
        return false;

    void *addr = krealloc(ht->entries, size);
    if(!addr)
        return false;

    ht->entries = addr;
    ht->entries_max = size;

    return true;
}
