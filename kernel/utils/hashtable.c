/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/utils/hashtable.h>
#include<dxgmx/crypto/sha1.h>
#include<dxgmx/stdlib.h>
#include<dxgmx/string.h>
#include<dxgmx/kmalloc.h>
#include<dxgmx/todo.h>

bool hashtable_init_sized(size_t entries, HashTable *ht)
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

bool hashtable_init(HashTable *ht)
{
    return hashtable_init_sized(4, ht); // idk, about 4 ?
}

void hashtable_add(size_t key, const void *data, HashTable *table)
{
    SHA1Digest digest = sha1_hash_buf((void*)&key, sizeof(key));

    size_t idx = (digest.h0 ^ digest.h1) + (digest.h2 ^ digest.h3) + ~digest.h4;
    idx %= table->entries_max;
    
    klogln(INFO, "%d", idx);

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
    {
        // if we got here the hash table is full.
        size_t newsize = table->entries_max * 2;
        table->entries = krealloc(table->entries, newsize);
        table->entries_max = newsize;
    }

    HashTableEntry *e = &table->entries[idx];
    e->key = key;
    e->value = data;
    e->used = true;
}

void *hashtable_get(size_t key, HashTable *table)
{
    SHA1Digest digest = sha1_hash_buf((void *)&key, sizeof(key));

    size_t idx = (digest.h0 ^ digest.h1) + (digest.h2 ^ digest.h3) + ~digest.h4;
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
