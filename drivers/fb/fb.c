/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifdef CONFIG_DEVFS
#include <dxgmx/devfs.h>
#include <dxgmx/posix/sys/stat.h>
#endif
#include <dxgmx/assert.h>
#include <dxgmx/err_or.h>
#include <dxgmx/errno.h>
#include <dxgmx/fb.h>
#include <dxgmx/fb_ioctl.h>
#include <dxgmx/fb_user.h>
#include <dxgmx/generated/kconfig.h>
#include <dxgmx/kboot.h>
#include <dxgmx/klog.h>
#include <dxgmx/mem/dma.h>
#include <dxgmx/mem/falloc.h>
#include <dxgmx/mem/mm.h>
#include <dxgmx/posix/sys/mman.h>
#include <dxgmx/proc/procm.h>
#include <dxgmx/sched/sched.h>
#include <dxgmx/string.h>
#include <dxgmx/user.h>

#define KLOGF_PREFIX "fb: "

static FrameBuffer g_fb;
static bool g_fb_up;

#ifdef CONFIG_DEVFS

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

    return user_copy_from(buf, (void*)(fb->vaddr + off), n);
}

static int fb_vnode_ioctl(VirtualNode* vnode, int req, void* data)
{
    if (!g_fb_up)
        return -EINVAL;

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
    res = dma_map_range(fb->paddr, len, map_flags, sched_current_proc());
    if (res.error)
        return (void*)res.error;

    return (void*)res.value;
}

static VirtualNodeOperations g_fb_vnode_ops = {
    .open = fb_vnode_open,
    .read = fb_vnode_read,
    .write = fb_vnode_write,
    .ioctl = fb_vnode_ioctl,
    .mmap = fb_vnode_mmap};

#endif // CONFIG_DEVFS

static int fb_init()
{
    ptr paddr = _kboot_framebuffer_paddr;
    size_t width = _kboot_framebuffer_width;
    size_t height = _kboot_framebuffer_height;
    size_t bpp = _kboot_framebuffer_bpp;

    if (paddr == 0)
        return -1;

    const size_t fb_size = width * height * (bpp / 8);

    ERR_OR(ptr)
    res = dma_map_range(paddr, fb_size, PAGE_RW, procm_get_kernel_proc());
    if (res.error < 0)
        return res.error;

    g_fb.paddr = paddr;
    g_fb.width = width;
    g_fb.height = height;
    g_fb.bpp = bpp;
    g_fb.bytespp = bpp / 8;
    g_fb.vaddr = res.value;
    g_fb_up = true;

#ifdef CONFIG_DEVFS
    devfs_register(
        "fb",
        S_IFREG | (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP),
        0,
        0,
        &g_fb_vnode_ops,
        &g_fb);
#endif

    return 0;
}

int fb_ensure_init()
{
    if (!g_fb_up)
        return fb_init();

    return 0;
}

int fb_write_pixel(size_t x, size_t y, u32 pixel)
{
    size_t off = y * g_fb.width * g_fb.bytespp + x * g_fb.bytespp;
    memcpy((void*)(g_fb.vaddr + off), &pixel, g_fb.bytespp);
    return 0;
}

FrameBuffer* fb_get_main()
{
    if (!g_fb_up)
        return NULL;

    return &g_fb;
}
