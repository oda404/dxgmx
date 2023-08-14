/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

// https://academy.cba.mit.edu/classes/networking_communications/SD/FAT.pdf

#include "fat.h"
#include <dxgmx/assert.h>
#include <dxgmx/ctype.h>
#include <dxgmx/errno.h>
#include <dxgmx/fs/vfs.h>
#include <dxgmx/fs/vnode.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/math.h>
#include <dxgmx/module.h>
#include <dxgmx/storage/blkdevm.h>
#include <dxgmx/string.h>
#include <dxgmx/todo.h>
#include <posix/sys/stat.h>

#define KLOGF_PREFIX "fatfs: "

#define MODULE_NAME "fatfs"

/**
 * Get the fat entry for a cluster.
 *
 * 'clus' The cluster.
 * 'buf' Buffer big enough to store a disk sector (for FAT reading).
 * 'ctx' The ctx.
 *
 * Returns:
 * A positive number representing the FAT entry on success.
 * A negative errno on error.
 */
static i64 fat_entry_for_clus(u32 clus, u8* buf, const FAT32Ctx* ctx)
{
    const MountableBlockDevice* part = ctx->blkdev;
    ASSERT(part);

    const u32 fat_offset = ctx->type == FAT32 ? clus * 4 : clus * 2;

    const u32 sec = ctx->reserved_sector_count + (fat_offset / ctx->sectorsize);
    const u32 off = fat_offset % ctx->sectorsize;

    int st = part->read(part, sec, 1, buf);
    if (st < 0)
        return st;

    return *((u32*)(buf + off));
}

static int fat_parse_metadata(FileSystem* fs)
{
    FAT32Ctx* ctx = FAT32_CTX(fs);

    const MountableBlockDevice* part = ctx->blkdev;
    ASSERT(part);

    u8* buf = kmalloc(part->sectorsize);
    if (!buf)
        return -ENOMEM;

    int st = part->read(part, 0, 1, buf);
    if (st < 0)
    {
        kfree(buf);
        return st;
    }

    const FatBootRecord* bpb = (FatBootRecord*)buf;
    const Fat32BootRecord* bpb32 =
        (const Fat32BootRecord*)((ptr)bpb + sizeof(FatBootRecord));

    /* Set the sectors per fat */
    if (bpb->sectors_per_fat16 == 0)
        ctx->sectors_per_fat = bpb32->sectors_per_fat32;
    else
        ctx->sectors_per_fat = bpb->sector_count16;

    ctx->sectorsize = bpb->sectorsize;
    /* Sector count is the biggest out of the sector_count16 or 32. */
    ctx->sector_count = max(bpb->sector_count16, bpb->sectors_count32);

    /* How many sectors the root directories occupy. */
    ctx->root_dir_sector_count =
        ((bpb->root_dir_entries_count * 32) + (bpb->sectorsize - 1)) /
        bpb->sectorsize;

    /* Most likely 1 */
    ctx->sectors_per_cluster = bpb->sectors_per_cluster;

    /* Calculate how many data sectors there are. */
    ctx->data_sector_count =
        ctx->sector_count -
        (bpb->reserved_sector_count + bpb->fat_count * ctx->sectors_per_fat +
         ctx->root_dir_sector_count);

    /* Clusters are just groups of sectors that hold file/dir data. */
    ctx->cluster_count = ctx->data_sector_count / ctx->sectors_per_cluster;

    ctx->reserved_sector_count = bpb->reserved_sector_count;

    ctx->first_data_sector = ctx->reserved_sector_count +
                             (bpb->fat_count * ctx->sectors_per_fat) +
                             ctx->root_dir_sector_count;

    if (ctx->cluster_count < 4085)
    {
        ctx->root_dir_cluster = 0;
        ctx->type = FAT12;
    }
    else if (ctx->cluster_count < 65525)
    {
        ctx->root_dir_cluster = 0;
        ctx->type = FAT16;
    }
    else
    {
        ctx->root_dir_cluster = bpb32->root_dir_cluster;
        ctx->type = FAT32;
    }

    ctx->clustersize = ctx->sectors_per_cluster * ctx->sectorsize;

    ctx->entries_per_clusterdir = ctx->clustersize / 32;

    kfree(buf);
    return true;
}

static int fat_validate(const MountableBlockDevice* blkdev)
{
    u8* buf = kmalloc(blkdev->sectorsize);
    if (!buf)
        return -ENOMEM;

    int st = blkdev->read(blkdev, 0, 1, buf);
    if (st < 0)
    {
        kfree(buf);
        return st;
    }

    const FatBootRecord* bpb = (FatBootRecord*)buf;
    const Fat32BootRecord* bpb32 =
        (const Fat32BootRecord*)((ptr)bpb + sizeof(FatBootRecord));

    /* Something is very wrong. */
    if (bpb->sectorsize != blkdev->sectorsize)
    {
        kfree(buf);
        return -EINVAL;
    }

    st = -EINVAL;
    /* Idk if there is a better way of figuring out if a volume is
    FAT, but this seems to work for now. */
    if ((bpb->sectors_per_fat16 == 0 && bpb32->sectors_per_fat32) ||
        bpb->sectors_per_fat16)
    {
        if (bpb->sectorsize == 512 || bpb->sectorsize == 1024 ||
            bpb->sectorsize == 2048 || bpb->sectorsize == 4096)
        {
            st = 0;
        }
    }

    kfree(buf);
    return st;
}

static void fat_destroy(FileSystem* fs)
{
    /* This driver allocates just two things, it's metadata, and vnodes. */
    if (fs->driver_ctx)
    {
        kfree(fs->driver_ctx);
        fs->driver_ctx = NULL;
    }

    FOR_EACH_ENTRY_IN_LL (fs->vnode_ll, VirtualNode*, vnode)
        kfree(vnode->name);

    fs_free_all_cached_vnodes(fs);
}

static int fat_init(FileSystem* fs)
{
    /* Find the block device. */
    const MountableBlockDevice* blkdev =
        blkdevm_find_mountable_blkdev(fs->mntsrc);
    if (!blkdev)
        return -ENOTBLK;

    int st = fat_validate(blkdev);
    if (st < 0)
        return st;

    /* Allocate the FAT32 ctx. */
    FAT32Ctx* fatctx = kmalloc(sizeof(FAT32Ctx));
    if (!fatctx)
        return -ENOMEM;

    fatctx->blkdev = blkdev;
    fs->driver_ctx = fatctx;

    st = fat_parse_metadata(fs);
    if (st < 0)
    {
        fat_destroy(fs);
        return st;
    }

    /* We don't support FAT 12/16, if you want it, feel free to implement it */
    if (fatctx->type != FAT32)
    {
        fat_destroy(fs);
        return -EINVAL;
    }

    st = fat_cache_root_vnode(fs);
    if (st < 0)
    {
        fat_destroy(fs);
        return st;
    }

    st = fat_enumerate_and_cache_dir(fs_lookup_vnode("/", fs), fs);
    if (st < 0)
    {
        fat_destroy(fs);
        return st;
    }

    return 0;
}

static int fat_ioctl(VirtualNode*, int, void*)
{
    return -1;
}

static ssize_t
fat_write(VirtualNode* vnode, const void* buf, size_t n, off_t off)
{
    (void)vnode;
    (void)buf;
    (void)n;
    (void)off;
    TODO_FATAL();
}

static ERR_OR(ino_t) fat_mkfile(
    VirtualNode* dir,
    const char* name,
    mode_t mode,
    uid_t uid,
    gid_t gid,
    FileSystem* fs)
{
    (void)fs;
    (void)mode;
    (void)uid;
    (void)gid;
    (void)name;
    (void)dir;
    TODO_FATAL();
}

static const VirtualNodeOperations g_fatfs_vnode_ops = {
    .read = fat_read, .write = fat_write, .ioctl = fat_ioctl};

static const FileSystemDriver g_fatfs_driver = {
    .name = MODULE_NAME,
    .generic_probe = true,
    .init = fat_init,
    .destroy = fat_destroy,
    .mkfile = fat_mkfile,
    .vnode_ops = &g_fatfs_vnode_ops};

static int fatfs_main()
{
    return vfs_register_fs_driver(&g_fatfs_driver);
}

static int fatfs_exit()
{
    return vfs_unregister_fs_driver(&g_fatfs_driver);
}

MODULE g_fat_module = {
    .name = MODULE_NAME,
    .main = fatfs_main,
    .stage = MODULE_STAGE3,
    .exit = fatfs_exit};

/* FAT32 utilities. */

int fat_cluster_type(u32 cluster, u8* buf, const FAT32Ctx* ctx)
{
    i64 fat = fat_entry_for_clus(cluster, buf, ctx);
    if (fat < 0)
        return fat;

    if (fat == 0)
        return FAT_CLUSTER_FREE;

    if (fat >= 0 && fat <= ctx->cluster_count)
        return FAT_CLUSTER_USED;

    if (fat > ctx->cluster_count && fat <= 0xFFFFFF6)
        return FAT_CLUSTER_RESERVED;

    if (fat == 0xFFFFFF7)
        return FAT_CLUSTER_BAD;

    if (fat >= 0xFFFFFF8)
        return FAT_CLUSTER_EOF;

    return FAT_CLUSTER_BROKEN;
}

int fat_read_one_cluster(u8* buf, u32 clus, const FAT32Ctx* ctx)
{
    const MountableBlockDevice* part = ctx->blkdev;
    ASSERT(part);

    /* Try to read the first cluster sector */
    const u32 sector = fat_cluster_to_sector(clus, ctx);

    return part->read(part, sector, ctx->sectors_per_cluster, buf);
}

void fat_advance_loc(const FAT32Ctx* ctx, FATEntryLoc* loc)
{
    if (loc->offset + 1 == ctx->entries_per_clusterdir)
    {
        loc->offset = 0;
        loc->read_next = true;
    }
    else
    {
        ++loc->offset;
        loc->read_next = false;
    }
}

int fat_advance_loc_clus(FATEntryLoc* loc, u8* buf, const FAT32Ctx* ctx)
{
    i64 next_cluster = fat_entry_for_clus(loc->cluster, buf, ctx);
    if (next_cluster < 0)
        return next_cluster;

    /* If the fat entry (next cluster) is out of this range, it could either be
     * an EOF, reserved, corrupted or free. We only care that those can't be
     * used. */
    if (next_cluster >= 2 && next_cluster <= ctx->cluster_count)
    {
        loc->cluster = next_cluster;
        loc->read_next = false;
        return 0;
    }

    return -EINVAL;
}

void fat_read_one_entry(u8* clustersrc, size_t idx, FATEntry* entry)
{
    memcpy(entry, clustersrc + idx * 32, sizeof(FATEntry));
}

int fat_free_vnode(VirtualNode* vnode)
{
    kfree(vnode->name);
    vnode->name = NULL;
    return 0;
}

int fat_next_clus(u32* clus, u8* buf, const FAT32Ctx* ctx)
{
    u32 next_clus = fat_entry_for_clus(*clus, buf, ctx);
    if (next_clus >= 2 && next_clus <= ctx->cluster_count)
    {
        *clus = next_clus;
        return 0;
    }

    return -EINVAL;
}

u32 fat_cluster_to_sector(u32 cluster, const FAT32Ctx* ctx)
{
    return (cluster - 2) * ctx->sectors_per_cluster + ctx->first_data_sector;
}
