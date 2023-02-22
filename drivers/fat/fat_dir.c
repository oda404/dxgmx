/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include "fat.h"
#include <dxgmx/ctype.h>
#include <dxgmx/errno.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/string.h>

#define KLOGF_PREFIX "fat_dir: "

static bool fat_entry_is_last_in_dir(FATEntry* entry)
{
    return entry->name[0] == 0;
}

/**
 * An entry that was deleted may still be present on disk until it is
 * overwritten by fresh data.
 */
static bool fat_is_entry_deleted(FATEntry* entry)
{
    /* 0xE5 is also a kanji character if the volume uses a jap characted set...
     * Oh well. */
    return (u8)entry->name[0] == 0xE5;
}

static bool fat_is_dot_or_dotdot(char name[11])
{
    if (name[1] == ' ')
        return name[0] == '.';

    if (name[2] == ' ')
        return name[0] == '.' && name[1] == '.';

    return false;
}

/**
 * Get the size of a legacy FAT32 filename (a name that doesn't use LFNs).
 *
 * 'entry' The entry.
 *
 * Returns:
 * The filename size including the NULL terminator.
 */
static u8 fat_legacy_name_size(FATEntry* entry)
{
    size_t size = 0;
    for (size_t i = 0; i < 8; ++i)
    {
        if (entry->name[i] == ' ')
            break;

        ++size;
    }

    bool has_ext = false;
    for (size_t i = 0; i < 3; ++i)
    {
        if (entry->ext[i] == ' ')
            break;

        ++size;
        has_ext = true;
    }

    if (has_ext)
        ++size; // the dot.

    return size + 1;
}

/**
 * Copy a legacy FAT32 filename (a name that doesn't use LFNs) into a dest
 * buffer.
 *
 * 'entry' The entry from which to copy.
 * 'dest' A buffer big enough to hold a FAT32 legacy filename, check
 * fat_legacy_name_size().
 */
static void fat_copy_legacy_name(FATEntry* entry, char* dest)
{
    char name[9] = {0};
    char ext[4] = {0};
    bool has_ext = false;

    for (size_t i = 0; i < 8; ++i)
    {
        if (entry->name[i] == ' ')
            break;

        name[i] =
            isupper(entry->name[i]) ? tolower(entry->name[i]) : entry->name[i];
    }

    for (size_t i = 0; i < 3; ++i)
    {
        if (entry->ext[i] == ' ')
            break;

        ext[i] =
            isupper(entry->ext[i]) ? tolower(entry->ext[i]) : entry->ext[i];

        has_ext = true;
    }

    strcpy(dest, name);
    if (has_ext)
    {
        strcat(dest, ".");
        strcat(dest, ext);
    }
}

/**
 * Get the size of a filename constructed from LFN entries. This function may
 * return a bigger number than the actual filename size, as it only gets the
 * number of LFN entries and multiplies them by 13, while the actual name may be
 * a bit smaller (but not by a lot).
 *
 * 'lfn_entry' The first entry.
 *
 * Returns:
 * The filename size including the NULL terminator.
 */
static size_t fat_lfn_name_size(FATLFNEntry* lfn_entry)
{
    const size_t lfn_entries = lfn_entry->order & ~(0x40);
    return lfn_entries * 13 + 1;
}

/**
 * Get the total number of LFN entries, starting from the first entry.
 *
 * 'lfn_entry' The first entry.
 *
 * Returns:
 * The total number of LFN entries for this file.
 */
static size_t fat_lfn_entries_count(FATLFNEntry* lfn_entry)
{
    return lfn_entry->order & ~(0x40);
}

/**
 * Read all Long File Name entries starting from the first one.
 *
 * 'clusterbuf' Buffer big enough to hold a cluster (for storing and parsing
 * sectors).
 * 'fatbuf' Buffer big enough to hold one disk sector (for FAT
 * reading).
 * 'loc' Starting location. Will get updated to point at the next valid entry,
 * once all LFNs are have been read.
 * 'ctx' The ctx.
 * 'dest' The name destination buffer.
 * 'FatEntry' The stating entry.
 *
 * Returns:
 * 0 on success.
 * -EINVAL if the filesystem did something unexpected (very possibly corrupted).
 * Other negative errnos values coming from the disk.
 */
static ssize_t fat_read_lfns(
    u8* clusterbuf,
    u8* fatbuf,
    FATEntryLoc* loc,
    const FAT32Ctx* ctx,
    char* dest,
    FATEntry* entry)
{
    const size_t total_lfns = fat_lfn_entries_count((FATLFNEntry*)entry);
    size_t lfn_entries_read = 0;

    while (entry->attributes == FAT_ENTRY_LFN)
    {
        if (lfn_entries_read >= total_lfns)
            return -EINVAL;

        FATLFNEntry* lfn_entry = (FATLFNEntry*)entry;
        size_t i = 0;

        for (size_t k = 0; k < 5; ++k)
        {
            u16 c = lfn_entry->name1[k];
            if (c == 0 || c == 0xFFFF)
                break;
            dest[(total_lfns - lfn_entries_read - 1) * 13 + i++] = c & 0xFF;
        }

        for (size_t k = 0; k < 6; ++k)
        {
            u16 c = lfn_entry->name2[k];
            if (c == 0 || c == 0xFFFF)
                break;
            dest[(total_lfns - lfn_entries_read - 1) * 13 + i++] = c & 0xFF;
        }

        for (size_t k = 0; k < 2; ++k)
        {
            u16 c = lfn_entry->name3[k];
            if (c == 0 || c == 0xFFFF)
                break;
            dest[(total_lfns - lfn_entries_read - 1) * 13 + i++] = c & 0xFF;
        }

        /* Read a new cluster if required. */
        if (loc->read_next)
        {
            /* If this fails it we needed to read the next cluster, but the
             * previous one was the last. */
            int st = fat_advance_loc_clus(loc, fatbuf, ctx);
            if (st < 0)
                return st;

            /* I don't think this is needed, REALLY make sure we are reading
             * from a good cluster. */
            int type = fat_cluster_type(loc->cluster, fatbuf, ctx);
            if (!(type == FAT_CLUSTER_USED || type == FAT_CLUSTER_EOF))
                return -EINVAL;

            st = fat_read_one_cluster(clusterbuf, loc->cluster, ctx);
            if (st < 0)
                return st;
        }

        fat_read_one_entry(clusterbuf, loc->offset, entry);

        /* Advance by one */
        fat_advance_loc(ctx, loc);

        ++lfn_entries_read;
    }

    return 0;
}

/**
 * Transform a FATEntryLoc from a directory cluster into a VirtualNode. This
 * function modifies 'loc' to point to the next entry in the directory (if any,
 * check returns).
 *
 * 'loc' The location from which to start.
 * 'vnode' The vnode to fill. 'owner', 'parent' and 'ops' should be set
 * before calling this function.
 *
 * Returns:
 * 0 if the vnode has been successfully set, and there are more entries in the
 * dir.
 * -EAGAIN if the entry has been skipped, because it's not relevant, and there
 * are still more entries to check in the dir.
 * -ENOENT if there are no more entries in the dir.
 * -ENOMEM on out of memory.
 * -EINVAL if the filesystem did something unexpected (very possibly corrupted).
 */
static int fat_dir_entry_to_vnode(FATEntryLoc* loc, VirtualNode* vnode)
{
    const FAT32Ctx* ctx = FAT32_CTX(vnode->owner);

    u8* fatbuf = kmalloc(ctx->blkdev->sectorsize);
    if (!fatbuf)
        return -ENOMEM;

    if (loc->read_next)
    {
        /* If this fails it we needed to read the next cluster, but the previous
         * one was the last. */
        int st = fat_advance_loc_clus(loc, fatbuf, ctx);
        if (st < 0)
        {
            kfree(fatbuf);
            return st;
        }

        /* I don't think this is needed, REALLY make sure we are reading from a
         * good cluster. */
        int clus_type = fat_cluster_type(loc->cluster, fatbuf, ctx);
        if (!(clus_type == FAT_CLUSTER_USED || clus_type == FAT_CLUSTER_EOF))
        {
            kfree(fatbuf);
            return -EINVAL;
        }
    }

    /* Allocate buffer for one cluster */
    u8* clusbuf = kmalloc(ctx->clustersize);
    if (!clusbuf)
    {
        kfree(fatbuf);
        return -ENOMEM;
    }

    /* Read one cluster.*/
    int st = fat_read_one_cluster(clusbuf, loc->cluster, ctx);
    if (st < 0)
        goto out_err;

    /* Make sense of that cluster by reading one entry from it. */
    FATEntry entry;
    fat_read_one_entry(clusbuf, loc->offset, &entry);

    /* If this is the last entry in the dir cluster, we signal that to the
     * caller. */
    if (fat_entry_is_last_in_dir(&entry))
    {
        st = -ENOENT;
        goto out_err;
    }

    /* If this entry exists (is valid), advance to the next entry. Note that we
     * don't yet read that entry. */
    fat_advance_loc(ctx, loc);

    /*  We ignore those and ./ and ../ */
    if (fat_is_entry_deleted(&entry) || fat_is_dot_or_dotdot(entry.name))
    {
        st = -EAGAIN;
        goto out_err;
    }

    if (entry.attributes == FAT_ENTRY_LFN)
    {
        size_t namesize = fat_lfn_name_size((FATLFNEntry*)&entry);
        vnode->name = kcalloc(namesize);
        if (!vnode->name)
        {
            st = -ENOMEM;
            goto out_err;
        }

        st = fat_read_lfns(clusbuf, fatbuf, loc, ctx, vnode->name, &entry);
        if (st < 0)
        {
            kfree(vnode->name);
            goto out_err;
        }
    }
    else if (entry.attributes == FAT_ENTRY_VOL_ID)
    {
        st = -EAGAIN;
        goto out_err;
    }

    vnode->n = ((u32)entry.cluster_hi) << 16 | entry.cluster_lo;
    vnode->mode =
        (entry.attributes & FAT_ENTRY_DIR) ? FAT_DIR_MODE : FAT_FILE_MODE;

    /* vnode->name is already set if we had lfns */
    if (!vnode->name)
    {
        vnode->name = kmalloc(fat_legacy_name_size(&entry));
        if (!vnode->name)
        {
            goto out_err;
            st = -ENOMEM;
        }

        fat_copy_legacy_name(&entry, vnode->name);
    }

    vnode->size = entry.file_size;
    vnode->state = 0;

    kfree(fatbuf);
    kfree(clusbuf);
    return 0;

out_err:
    kfree(fatbuf);
    kfree(clusbuf);
    return st;
}

int fat_enumerate_and_cache_dir(const VirtualNode* dir_vnode, FileSystem* fs)
{
    ASSERT((dir_vnode->mode & S_IFMT) == S_IFDIR);

    FATEntryLoc loc = {.cluster = dir_vnode->n, .offset = 0};

    while (true)
    {
        VirtualNode vnode = {0};
        vnode.owner = fs;
        vnode.parent = dir_vnode;
        vnode.ops = fs->driver->vnode_ops;

        int st = fat_dir_entry_to_vnode(&loc, &vnode);
        if (st == -EAGAIN)
            continue; // Entry skipped
        else if (st == -ENOENT)
            break; // No more entries.
        else if (st < 0)
            return st; // Error occurred

        VirtualNode* cached_vnode = fs_new_vnode_cache(fs);
        if (!cached_vnode)
        {
            fat_free_vnode(&vnode);
            /* Let's be nice and at least try to enumerate any remaining
             * entries, maybe something will free up in the mean time. */
            continue;
        }

        *cached_vnode = vnode;

        /* I love recursion (not) */
        if ((cached_vnode->mode & S_IFMT) == S_IFDIR)
            fat_enumerate_and_cache_dir(cached_vnode, fs);
    }

    return 0;
}

int fat_cache_root_vnode(FileSystem* fs)
{
    FAT32Ctx* fatctx = FAT32_CTX(fs);

    VirtualNode* vnode = fs_new_vnode_cache(fs);
    if (!vnode)
        return -ENOMEM;

    vnode->n = fatctx->root_dir_cluster;
    vnode->parent = NULL;
    vnode->mode = FAT_DIR_MODE;
    vnode->size = 0;

    vnode->name = kmalloc(2);
    if (!vnode->name)
    {
        fs_free_cached_vnode(vnode, fs);
        return -ENOMEM;
    }
    vnode->name[0] = '/';
    vnode->name[1] = '\0';

    vnode->state = 0;

    return 0;
}
