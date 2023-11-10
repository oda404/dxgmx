/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/err_or.h>
#include <dxgmx/errno.h>
#include <dxgmx/fb.h>
#include <dxgmx/generated/kconfig.h>
#include <dxgmx/kboot.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/mem/dma.h>
#include <dxgmx/module.h>
#include <dxgmx/proc/procm.h>
#include <dxgmx/string.h>
#include <dxgmx/todo.h>

#ifdef CONFIG_DEVFS
#include <dxgmx/fb_devfs.h>
#endif

static FrameBuffer* g_fbs;
static size_t g_fb_count;
static FrameBufferPack g_fbpack = {.fbs = &g_fbs, .fb_count = &g_fb_count};

static FrameBuffer* fb_new_framebuffer()
{
    FrameBuffer* tmp = krealloc(g_fbs, (g_fb_count + 1) * sizeof(FrameBuffer));
    if (!tmp)
        return NULL;

    g_fbs = tmp;
    ++g_fb_count;
    return &g_fbs[g_fb_count - 1];
}

static void fb_destroy_framebuffer(FrameBuffer* fb)
{
    for (FrameBuffer* f = fb; f < g_fbs + g_fb_count - 1; ++f)
        *f = *(f + 1);

    --g_fb_count;

    if (!g_fb_count)
    {
        kfree(g_fbs);
        g_fbs = NULL;
    }
    else
    {
        g_fbs = krealloc(g_fbs, g_fb_count * sizeof(FrameBuffer));
    }
}

static int fb_init()
{
    if (!___kboot_info.has_fb)
        return -ENOENT;

    const ptr paddr = ___kboot_info.fb_pa;
    const size_t width = ___kboot_info.fb_width;
    const size_t height = ___kboot_info.fb_height;
    const size_t bpp = ___kboot_info.fb_bpp;
    const size_t fb_size = width * height * (bpp / 8);

    FrameBuffer* fb = fb_new_framebuffer();
    if (!fb)
        return -ENOMEM;

    ERR_OR(ptr)
    res = dma_map_range(paddr, fb_size, PAGE_RW, procm_get_kernel_proc());
    if (res.error < 0)
    {
        fb_destroy_framebuffer(fb);
        return res.error;
    }

    fb->base_pa = paddr;
    fb->width = width;
    fb->height = height;
    fb->bpp = bpp;
    fb->bytespp = bpp / 8;
    fb->base_va = res.value;

#ifdef CONFIG_DEVFS
    fb_devfs_register(0, fb);
#endif
    return 0;
}

FrameBufferPack* fb_get_pack()
{
    return &g_fbpack;
}

int fb_write_pixel(size_t x, size_t y, u32 pixel, FrameBuffer* fb)
{
    size_t off = y * fb->width * fb->bytespp + x * fb->bytespp;
    memcpy((void*)(fb->base_va + off), &pixel, fb->bytespp);
    return 0;
}

static int fb_main()
{
    return fb_init();
}

static int fb_exit()
{
    TODO();
    return 0;
}

MODULE g_fb_module = {.name = "fb", .main = fb_main, .exit = fb_exit};
