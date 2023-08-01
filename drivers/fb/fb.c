/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifdef CONFIG_DEVFS
#include <dxgmx/devfs.h>
#include <posix/sys/stat.h>
#endif
#include <dxgmx/fb.h>
#include <dxgmx/kboot.h>
#include <dxgmx/klog.h>
#include <dxgmx/mem/dma.h>
#include <dxgmx/mem/falloc.h>
#include <dxgmx/mem/mm.h>
#include <dxgmx/sched/sched.h>
#include <dxgmx/string.h>

#define KLOGF_PREFIX "fb: "

static FrameBuffer g_fb;
static bool g_fb_up;

static ERR_OR(ptr) fb_map_to_virtual_space(ptr paddr, size_t n)
{
    return dma_map_range(paddr, n, PAGE_R | PAGE_W);
}

static ssize_t
fb_vnode_read(const VirtualNode* vnode, void* buf, size_t n, loff_t off)
{
    KLOGF(INFO, "fb read");
    return 0;
}

static ssize_t
fb_vnode_write(VirtualNode* vnode, const void* buf, size_t n, loff_t off)
{
    KLOGF(INFO, "fb write");
    return 0;
}

VirtualNodeOperations g_fb_vnode_ops = {
    .read = fb_vnode_read, .write = fb_vnode_write};

static int fb_init()
{
    ptr paddr = _kboot_framebuffer_paddr;
    size_t width = _kboot_framebuffer_width;
    size_t height = _kboot_framebuffer_height;
    size_t bpp = _kboot_framebuffer_bpp;

    if (paddr == 0)
        return -1;

    const size_t fb_size = width * height * (bpp / 8);

    ERR_OR(ptr) res = fb_map_to_virtual_space(paddr, fb_size);
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
        &g_fb_vnode_ops);
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
