/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include "fat.h"
#include <dxgmx/errno.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/math.h>
#include <dxgmx/string.h>

ssize_t fat_read(const VirtualNode* vnode, void* buf, size_t n, off_t off)
{
    if (n == 0 || (size_t)off >= vnode->size)
        return 0;

    FileSystem* fs = vnode->owner;

    const FAT32Ctx* ctx = FAT32_CTX(fs);

    const MountableBlockDevice* part = ctx->blkdev;
    ASSERT(part);

    /* Cap n to the vnode size, taking into account the offset. */
    n = min(n, vnode->size - off);

    /* This buffer is passed to different functions that read a sector from
     * disk, and parse a fat entry, so those utilities don't allocate the buffer
     * themselves every time. */
    u8* fatbuf = kmalloc(ctx->sectorsize);
    if (!fatbuf)
        return -ENOMEM;

    u32 cluster = vnode->n;

    /* Jump clusters if the offset requires us to do so */
    size_t cluster_offset = off / ctx->clustersize;
    while (cluster_offset > 0)
    {
        int st = fat_next_clus(&cluster, fatbuf, ctx);
        if (st < 0)
        {
            kfree(fatbuf);
            return st;
        }

        --cluster_offset;
    }

    size_t cluster_alloc_size =
        (u32)ceil((double)n / ctx->clustersize) * ctx->clustersize;

    /* Allocate a buffer for reading ALL of our clusters. This can be done a lot
     * better by only allocating a buffer for one cluster, and reading them one
     * at a time... Oh well */
    u8* clusbuf = kmalloc(cluster_alloc_size);
    if (!clusbuf)
    {
        kfree(fatbuf);
        return -ENOMEM;
    }

    size_t working_size = n;
    size_t clusbuf_off = 0;
    /* Read multiple clusters if n requires us to do so. */
    while (working_size >= ctx->clustersize)
    {
        int st = part->read(
            part,
            fat_cluster_to_sector(cluster, ctx),
            ctx->sectors_per_cluster,
            clusbuf + clusbuf_off * ctx->clustersize);

        if (st < 0)
        {
            kfree(clusbuf);
            kfree(fatbuf);
            return st;
        }

        st = fat_next_clus(&cluster, fatbuf, ctx);
        if (st < 0)
        {
            kfree(clusbuf);
            kfree(fatbuf);
            return st;
        }

        working_size -= ctx->clustersize;
        ++clusbuf_off;
    }

    /* If the above loop didn't run, or we are still left with some data, read
     * another cluster */
    if (working_size > 0)
    {
        int st = part->read(
            part,
            fat_cluster_to_sector(cluster, ctx),
            ctx->sectors_per_cluster,
            clusbuf + clusbuf_off * ctx->clustersize);

        if (st < 0)
        {
            kfree(clusbuf);
            kfree(fatbuf);
            return st;
        }
    }

    memcpy(buf, clusbuf + off, n);

    kfree(fatbuf);
    kfree(clusbuf);
    return n;
}
