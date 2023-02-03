/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

// https://aboutitdata.files.wordpress.com/2020/07/fat32-spec-sda-contribution.pdf

#include "fat.h"
#include <dxgmx/assert.h>
#include <dxgmx/errno.h>
#include <dxgmx/fs/vfs.h>
#include <dxgmx/fs/vnode.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/math.h>
#include <dxgmx/module.h>
#include <dxgmx/storage/blkdevmanager.h>
#include <dxgmx/string.h>
#include <dxgmx/todo.h>
#include <posix/sys/stat.h>

#define KLOGF_PREFIX "fatfs: "

#define MODULE_NAME "fatfs"

#define FAT_FILE_MODE (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)
#define FAT_DIR_MODE (FAT_FILE_MODE | S_IFDIR)

typedef struct S_FatFs
{
    u16 sectorsize;
    /* How many sectors one FAT occupies. */
    u32 sectors_per_fat;
    /* How many sectors are in a cluster */
    u8 sectors_per_cluster;
    /* How many sectors there are on the volume. */
    u32 sectors_count;
    /* How many sectors there are in the data region. */
    u32 data_sectors_count;
    /* How many clusters there are on the volume. A cluster is a grouping
    of 'sectors_per_cluster' sectors that hold the data of a file/directory. */
    u32 clusters_count;
    /* How many sectors the root directories occupy */
    u16 root_dir_sectors_count;
    /* The first cluster of the root dir. */
    u32 root_dir_cluster;
    /* Includes BPB */
    u16 reserved_sectors_count;
    /*  */
    u32 first_data_sector;
    FatType type;

    const BlockDevice* blkdev;
} FatFs;

#define FAT_ENTRY_RO 0x01
#define FAT_ENTRY_HIDDEN 0x02
#define FAT_ENTRY_SYSTEM 0x04
#define FAT_ENTRY_VOL_ID 0x08
#define FAT_ENTRY_DIR 0x10
#define FAT_ENTRY_ARCHIVE 0x20
#define FAT_ENTRY_LFN 0x0F

typedef struct _ATTR_PACKED S_FatEntryTime
{
    u8 hour : 5;
    u8 min : 6;
    u8 sec_div2 : 5;
} FatEntryTime;

typedef struct _ATTR_PACKED S_FatEntryDate
{
    u8 year : 7;
    u8 month : 4;
    u8 day : 5;
} FatEntryDate;

typedef struct _ATTR_PACKED S_FatEntry
{
    char name[8];
    char ext[3];
    u8 attributes;
    u8 ntreserved;
    u8 creation_tenths_of_sec;
    FatEntryTime creation_time;
    FatEntryDate creation_date;
    FatEntryDate last_access_date;
    u16 cluster_hi;
    FatEntryDate last_write_time;
    FatEntryDate last_write_date;
    u16 cluster_lo;
    u32 file_size;
} FatEntry;

typedef struct _ATTR_PACKED S_FatLfnEntry
{
    u8 order;
    u16 name1[5];
    u8 attributes;
    u8 type;
    u8 checksum;
    u16 name2[6];
    u16 reserved;
    u16 name3[2];
} FatLfnEntry;

typedef struct S_FatEntryAddress
{
    u32 cluster;
    u8 offset;
} FatEntryAddress;

/* Returns the sector and offset into that sector for the FAT entry which
corresponds to the given cluster. */
static _ATTR_ALWAYS_INLINE FatEntryAddress
fat_get_fat_entry_addr_for_clust(u32 clust, const FatFs* fatfs)
{
    const u32 fat_offset = fatfs->type == FAT32 ? clust * 4 : clust * 2;
    return (FatEntryAddress){
        .cluster =
            fatfs->reserved_sectors_count + (fat_offset / fatfs->sectorsize),
        .offset = fat_offset % fatfs->sectorsize};
}

static i64 fat_get_fat_entry_for_clust(u32 clust, const FileSystem* fs)
{
    if (!fs)
        return -EINVAL;

    const FatFs* fatfs = fs->driver_ctx;
    const BlockDevice* part = fatfs->blkdev;
    ASSERT(part);

    // FIX returns.
    u8* tmp = kmalloc(part->physical_sectorsize);
    if (!tmp)
        return -ENOMEM;

    const FatEntryAddress loc =
        fat_get_fat_entry_addr_for_clust(clust, fs->driver_ctx);

    if (!part->read(part, loc.cluster, 1, tmp))
    {
        kfree(tmp);
        return -EIO;
    }

    const u32 ret = *((u32*)(tmp + loc.offset));
    kfree(tmp);
    return ret;
}

static _ATTR_ALWAYS_INLINE u32
fat_cluster_to_sector(u32 cluster, const FatFs* fatfs)
{
    return (cluster - 2) * fatfs->sectors_per_cluster +
           fatfs->first_data_sector;
}

/**
 * @brief Reads a FAT entry info, like it's name, data cluster ans size.
 * This doesn't actually read the file data.
 *
 * @param cluster The cluster from which to read.
 * @param entry_offset The offset into the cluster.
 * @param entry_out Where to put the entry info.
 * @param part The partition.
 * @return u32 How many 32byte entries have been read. (Multiple entries
 * can be read for one single file/directory because of LFNs).
 * If the value is < 0, it's an errno value:
 *   - EINVAL: An invalid argument was provided
 *   - ENOMEM: There was an error allocating memory
 *   - EIO:    There was an error when reading from the disk
 *   - ENOENT: The entry is not allocated (IE: end of cluster)
 *   - ENFILE:: The is either corrupted or bad clusters were encountered.
 */
static ssize_t fat_read_entry(
    FatEntryAddress loc, VirtualNode* vnode_out, const FileSystem* fs)
{
    if (loc.offset > 31)
        return -EINVAL;

    const FatFs* fatfs = fs->driver_ctx;
    ASSERT(fatfs);

    const BlockDevice* part = fatfs->blkdev;
    ASSERT(part);

    /* The buffer into which we read the cluster sectors */
    u8* buf = kmalloc(part->physical_sectorsize);
    if (!buf)
        return -ENOMEM;

    /* Try to read the first cluster sector */
    const u32 sector = fat_cluster_to_sector(loc.cluster, fatfs);
    if (!(part->read(part, sector, 1, buf)))
    {
        kfree(buf);
        return -EIO;
    }

    VirtualNode vnode;
    memset(&vnode, 0, sizeof(VirtualNode));

    u32 entries_read = 1;
    u32 idx = loc.offset;

    FatEntry fatentry;
    memcpy(&fatentry, buf + idx * 32, sizeof(fatentry));

    if (fatentry.name[0] == 0)
    {
        kfree(buf);
        return -ENOENT;
    }

    if ((u8)fatentry.name[0] == 0xE5)
        vnode.state |= INODE_STATE_WILL_FREE;

    bool is_lfn = false;
    if (fatentry.attributes == FAT_ENTRY_LFN)
    {
        FatLfnEntry* lfn_entry = (FatLfnEntry*)&fatentry;
        is_lfn = true;

        const size_t lfn_entries = lfn_entry->order & ~(0x40);
        vnode.name = kmalloc(lfn_entries * 13 + 1);
        if (!vnode.name)
        {
            kfree(buf);
            return -ENOMEM;
        }

        size_t itter = 0;
        while (fatentry.attributes == FAT_ENTRY_LFN &&
               entries_read <= lfn_entries)
        {
            for (size_t k = 0; k < 5; ++k)
            {
                u16 c = lfn_entry->name1[k];
                if (c == 0 || c == 0xFFFF)
                    break;
                vnode.name[itter++] = c & 0xFF;
                vnode.name[itter] = '\0';
            }
            for (size_t k = 0; k < 6; ++k)
            {
                u16 c = lfn_entry->name2[k];
                if (c == 0 || c == 0xFFFF)
                    break;
                vnode.name[itter++] = c & 0xFF;
                vnode.name[itter] = '\0';
            }

            for (size_t k = 0; k < 2; ++k)
            {
                u16 c = lfn_entry->name3[k];
                if (c == 0 || c == 0xFFFF)
                    break;
                vnode.name[itter++] = c & 0xFF;
                vnode.name[itter] = '\0';
            }

            /* If the next entry is on another cluster, we need to also
            read that cluster's sector. */
            if (++idx == 32)
            {
                idx = 0;
                const u32 clust = fat_get_fat_entry_for_clust(loc.cluster, fs);

                if (clust > 1 && clust <= fatfs->clusters_count)
                {
                    loc.cluster = clust;
                }
                else
                {
                    kfree(buf);
                    return -ENFILE;
                }

                const u32 sec = fat_cluster_to_sector(loc.cluster, fatfs);
                if (!(part->read(part, sec, 1, buf)))
                {
                    kfree(buf);
                    return -EIO;
                }
            }

            ++entries_read;
            memcpy(&fatentry, buf + idx * 32, 32);
        }
    }

    vnode.n = ((u32)fatentry.cluster_hi) << 16 | fatentry.cluster_lo;
    vnode.size = fatentry.file_size;
    if (!is_lfn)
    {
        vnode.name = kmalloc(12);
        if (!vnode.name)
        {
            kfree(buf);
            return -ENOMEM;
        }

        strcpy(vnode.name, fatentry.name);
        strcat(vnode.name, fatentry.ext);
    }

    vnode.mode =
        (fatentry.attributes & FAT_ENTRY_DIR) ? FAT_DIR_MODE : FAT_FILE_MODE;

    kfree(buf);
    *vnode_out = vnode;

    return entries_read;
}

static int fat_enumerate_dir(const VirtualNode* dir_vnode, FileSystem* fs)
{
    if (!dir_vnode || !fs)
        return -EINVAL;

    if ((dir_vnode->mode & S_IFMT) != S_IFDIR)
        return -EINVAL;

    const FatFs* fatfs = fs->driver_ctx;
    if (!fatfs)
        return -EINVAL;

    u32 cluster = dir_vnode->n;
    u32 offset = 0;

    while (true)
    {
        VirtualNode vnode;
        const ssize_t entries_read = fat_read_entry(
            (FatEntryAddress){.cluster = cluster, .offset = offset},
            &vnode,
            fs);

        if (entries_read < 0)
            break; //  we're done.

        offset += entries_read;

        const size_t entries_per_cluster =
            (fatfs->sectors_per_cluster * fatfs->sectorsize) / 32;

        while (offset >= entries_per_cluster)
        {
            offset -= entries_per_cluster;
            ++cluster;
        }

        /* Skip the current and previous directory */
        if (vnode.name)
        {
            if (*vnode.name == '.' || strcmp(vnode.name, "..") == 0)
                continue;
        }

        VirtualNode* tmp = fs_new_vnode(fs, vnode.n, dir_vnode);
        if (!tmp)
            return -ENOMEM;

        tmp->mode = vnode.mode;
        tmp->name = vnode.name;
        tmp->size = vnode.size;
        tmp->state = vnode.state;

        if (tmp->mode & S_IFDIR)
            fat_enumerate_dir(tmp, fs);
    }

    return 0;
}

static int fat_parse_metadata(FileSystem* fs)
{
    if (!fs)
        return -EINVAL;

    FatFs* fatfs = fs->driver_ctx;
    ASSERT(fatfs);

    const BlockDevice* part = fatfs->blkdev;
    ASSERT(part);

    u8* buf = kmalloc(part->physical_sectorsize);
    if (!buf)
        return -ENOMEM;

    if (!part->read(part, 0, 1, buf))
    {
        kfree(buf);
        return -EIO;
    }

    const FatBootRecord* bpb = (FatBootRecord*)buf;
    const Fat32BootRecord* bpb32 =
        (const Fat32BootRecord*)((ptr)bpb + sizeof(FatBootRecord));

    /* Set the sectors per fat */
    if (bpb->sectors_per_fat16 == 0)
        fatfs->sectors_per_fat = bpb32->sectors_per_fat32;
    else
        fatfs->sectors_per_fat = bpb->sectors_count16;

    fatfs->sectorsize = bpb->sectorsize;
    /* Sector count is the biggest out of the sector_count16 or 32. */
    fatfs->sectors_count = max(bpb->sectors_count16, bpb->sectors_count32);

    /* How many sectors the root directories occupy. */
    fatfs->root_dir_sectors_count =
        ((bpb->root_dir_entries_count * 32) + (bpb->sectorsize - 1)) /
        bpb->sectorsize;

    /* Most likely 1 */
    fatfs->sectors_per_cluster = bpb->sectors_per_cluster;

    /* Calculate how many data sectors there are. */
    fatfs->data_sectors_count =
        fatfs->sectors_count - (bpb->reserved_sectors_count +
                                bpb->fats_count * fatfs->sectors_per_fat +
                                fatfs->root_dir_sectors_count);

    /* Clusters are just groups of sectors that hold file/dir data. */
    fatfs->clusters_count =
        fatfs->data_sectors_count / fatfs->sectors_per_cluster;

    fatfs->reserved_sectors_count = bpb->reserved_sectors_count;

    fatfs->first_data_sector = fatfs->reserved_sectors_count +
                               (bpb->fats_count * fatfs->sectors_per_fat) +
                               fatfs->root_dir_sectors_count;

    if (fatfs->clusters_count < 4085)
    {
        fatfs->root_dir_cluster = 0;
        fatfs->type = FAT12;
    }
    else if (fatfs->clusters_count < 65525)
    {
        fatfs->root_dir_cluster = 0;
        fatfs->type = FAT16;
    }
    else
    {
        fatfs->root_dir_cluster = bpb32->root_dir_cluster;
        fatfs->type = FAT32;
    }

    kfree(buf);
    return true;
}

static int fat_valid(const BlockDevice* blkdev)
{
    if (!blkdev)
        return -EINVAL;

    u8* buf = kmalloc(blkdev->physical_sectorsize);
    if (!buf)
        return -ENOMEM;

    if (!blkdev->read(blkdev, 0, 1, buf))
    {
        kfree(buf);
        return -EIO;
    }

    const FatBootRecord* bpb = (FatBootRecord*)buf;
    const Fat32BootRecord* bpb32 =
        (const Fat32BootRecord*)((ptr)bpb + sizeof(FatBootRecord));

    bool assume_fat = false;
    /* Idk if there is a better way of figuring out if a volume is
    FAT, but this seems to work for now. */
    if ((bpb->sectors_per_fat16 == 0 && bpb32->sectors_per_fat32) ||
        bpb->sectors_per_fat16)
    {
        if (bpb->sectorsize == 512 || bpb->sectorsize == 1024 ||
            bpb->sectorsize == 2048 || bpb->sectorsize == 4096)
        {
            assume_fat = true;
        }
    }

    kfree(buf);

    return assume_fat;
}

static void fat_destroy(FileSystem* fs)
{
    if (fs->mntsrc)
    {
        kfree(fs->mntsrc);
        fs->mntsrc = NULL;
    }

    if (fs->driver_ctx)
    {
        kfree(fs->driver_ctx);
        fs->driver_ctx = NULL;
    }

    if (fs->vnodes)
    {
        kfree(fs->vnodes);
        fs->vnodes = NULL;
        fs->vnode_count = 0;
    }
}

static int
fat_init(const char* src, const char* type, const char* args, FileSystem* fs)
{
    (void)type;
    (void)args;

    /* This driver requires a src. */
    if (!fs || !src)
        return -EINVAL;

    /* If the type is explicitly NOT fat, it's not our business */
    if (type && strcmp(type, "fat") != 0)
        return -EINVAL;

    /* Find the block device using src */
    const BlockDevice* blkdev = blkdevm_find_blkdev_by_id(src);
    if (!blkdev)
        return -ENOTBLK;

    /* Allocate the Fat filesystem meta. */
    FatFs* fatfs = kmalloc(sizeof(FatFs));
    if (!fatfs)
        return -ENOMEM;

    /* Allocat any other variables we need to. */
    fs->mntsrc = strdup(src);
    if (!fs->mntsrc)
    {
        fat_destroy(fs);
        return -ENOMEM;
    }

    fs->driver_ctx = fatfs;
    fatfs->blkdev = blkdev;

    {
        int st = fat_parse_metadata(fs);
        if (st < 0)
        {
            fat_destroy(fs);
            return st;
        }
    }

    VirtualNode* rootvnode = fs_new_vnode(fs, fatfs->root_dir_cluster, NULL);
    if (!rootvnode)
    {
        fat_destroy(fs);
        return -ENOMEM;
    }

    rootvnode->mode = FAT_DIR_MODE;
    rootvnode->size = 0;
    rootvnode->name = "/";

    fat_enumerate_dir(rootvnode, fs);

    return 0;
}

static ssize_t fat_read(
    FileSystem* fs, const VirtualNode* vnode, void* buf, size_t n, loff_t off)
{
    if (n == 0)
        return 0; // as per spec

    if (!(fs && vnode && buf) || off >= vnode->size)
        return -EINVAL;

    /* If the vnode is not part of this filesystem we fucked up. */
    if (vnode->owner != fs)
        return -EINVAL;

    const FatFs* fatfs = fs->driver_ctx;
    ASSERT(fatfs);

    const BlockDevice* part = fatfs->blkdev;
    ASSERT(part);

    if (!(part && fatfs))
        return -EINVAL;

    /* Cap n to the vnode size, taking into account the offset. */
    n = min(n, vnode->size - off);

    const size_t sectors = ceil((double)n / part->physical_sectorsize);

    /* FIXME: This is a weird way to do this. */
    u8* tmpbuf = kmalloc(sectors * part->physical_sectorsize);
    if (!tmpbuf)
        return -ENOMEM;

    // FIXME: wth is this error checking
    if (!part->read(
            part,
            fat_cluster_to_sector(vnode->n, fatfs) +
                off / part->physical_sectorsize,
            sectors,
            tmpbuf))
    {
        kfree(tmpbuf);
        return -EIO;
    }

    memcpy(buf, tmpbuf + off, n);

    kfree(tmpbuf);

    return n;
}

static ssize_t fat_write(
    FileSystem* fs, VirtualNode* vnode, const void* buf, size_t n, loff_t off)
{
    (void)fs;
    (void)vnode;
    (void)buf;
    (void)n;
    (void)off;
    TODO_FATAL();
}

static int fat_mkfile(FileSystem* fs, const char* path, mode_t mode)
{
    (void)fs;
    (void)path;
    (void)mode;
    TODO_FATAL();
}

static int fat_mkdir(FileSystem*, const char*, mode_t)
{
    TODO_FATAL();
}

static int fatfs_main()
{
    const FileSystemDriver fs_driver = {
        .name = MODULE_NAME,
        .valid = fat_valid,
        .init = fat_init,
        .destroy = fat_destroy,
        .read = fat_read,
        .write = fat_write,
        .mkfile = fat_mkfile,
        .mkdir = fat_mkdir};

    return vfs_register_fs_driver(fs_driver);
}

static int fatfs_exit()
{
    return vfs_unregister_fs_driver(MODULE_NAME);
}

static MODULE const Module g_fat_module = {
    .name = MODULE_NAME, .main = fatfs_main, .exit = fatfs_exit};
