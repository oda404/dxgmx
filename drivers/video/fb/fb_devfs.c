/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/devfs.h>
#include <dxgmx/errno.h>
#include <dxgmx/fb_devfs.h>
#include <dxgmx/mem/dma.h>
#include <dxgmx/posix/sys/mman.h>
#include <dxgmx/posix/sys/stat.h>
#include <dxgmx/proc/procm.h>
#include <dxgmx/stdio.h>
#include <dxgmx/user.h>
#include <dxgmx/user@fb.h>

static int fbio_get_info(VirtualNode* vnode, FBInfo* info)
{
    FrameBuffer* fb = vnode->data;
    ASSERT(fb);
    FBInfo tmp = {.width = fb->width, .height = fb->height, .bpp = fb->bpp};
    return user_copy_to(info, &tmp, sizeof(FBInfo));
}

static int fb_vnode_open(VirtualNode* vnode, int flags)
{
    (void)flags;

    FrameBuffer* fb = vnode->data;
    fb->takeover = true;
    return 0;
}

static ssize_t fb_vnode_read(const VirtualNode*, void*, size_t, off_t)
{
    return -1;
}

static ssize_t fb_vnode_write(
    VirtualNode* vnode, const void* _USERPTR buf, size_t n, off_t off)
{
    FrameBuffer* fb = vnode->data;
    ASSERT(fb);

    size_t fbsize = fb->width * fb->height * fb->bytespp;
    if (off + n > fbsize)
        return -ENOSPC; // ?

    return user_copy_from(buf, (void*)(fb->base_va + off), n);
}

static int fb_vnode_ioctl(VirtualNode* vnode, int req, void* data)
{
    switch (req)
    {
    case FBIO_GET_INFO:
        return fbio_get_info(vnode, data);

    default:
        return -EINVAL;
    }
}

static void* fb_vnode_mmap(
    VirtualNode* vnode, void* addr, size_t len, int prot, int flags, off_t off)
{
    (void)vnode;
    (void)addr;
    (void)len;
    (void)prot;
    (void)flags;
    (void)off;

    (void)flags;

    if (!len)
        return NULL;

    ASSERT(addr == 0); // FIXME
    ASSERT(off == 0);  // FIXME

    FBInfo fbinfo;
    fbio_get_info(vnode, &fbinfo);
    const size_t fblen = fbinfo.width * fbinfo.height * (fbinfo.bpp / 8);

    if (len > fblen)
        return (void*)E2BIG;

    FrameBuffer* fb = vnode->data;

    u16 map_flags = PAGE_USER;
    map_flags |= prot & PROT_READ ? PAGE_R : 0;
    map_flags |= prot & PROT_WRITE ? PAGE_W : 0;

    ERR_OR(ptr)
    res =
        dma_map_range(fb->base_pa, len, map_flags, procm_sched_current_proc());
    if (res.error)
        return 0; // FIXME

    return (void*)res.value;
}

static VirtualNodeOperations g_fb_vnode_ops = {
    .open = fb_vnode_open,
    .read = fb_vnode_read,
    .write = fb_vnode_write,
    .ioctl = fb_vnode_ioctl,
    .mmap = fb_vnode_mmap};

int fb_devfs_register(size_t n, FrameBuffer* fb)
{
    if (n > 9)
        return -E2BIG; // Cuz i'm a jackass

    char name[4];
    snprintf(name, 4, "fb%d", n);
    return devfs_register(
        name,
        S_IFREG | (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP),
        0,
        0,
        &g_fb_vnode_ops,
        fb);
}
