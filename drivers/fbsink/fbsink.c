/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include "psf.h"
#include <dxgmx/errno.h>
#include <dxgmx/klog.h>
#include <dxgmx/kstdio/kstdio.h>
#include <dxgmx/kstdio/sink.h>
#include <dxgmx/module.h>
#include <dxgmx/sched/sched.h>
#include <dxgmx/string.h>
#include <dxgmx/video/fb.h>

#define KLOGF_PREFIX "fbsink: "

typedef struct S_FrameBufferTextRenderingContext
{
    FrameBuffer* fb;

    size_t cx;
    size_t cy;

    size_t glyphs_per_row;
    size_t glyphs_per_col;
} FrameBufferTextRenderingContext;

static u32 fbsink_koutput_color2pixel(KOutputColor color)
{
    switch (color)
    {
    case KOUTPUT_BLACK:
        return 0x0;
    case KOUTPUT_RED:
        return 0x00FF0000;
    case KOUTPUT_GREEN:
        return 0x0000FF00;
    case KOUTPUT_YELLOW:
        return 0x00FFFF00;
    case KOUTPUT_BLUE:
        return 0x000000FF;
    case KOUTPUT_MAGENTA:
        return 0x00FF00FF;
    case KOUTPUT_CYAN:
        return 0x0000FFFF;

    default:
    case KOUTPUT_WHITE:
        return 0x00CCCCCC;
    }
}

static int fbsink_scroll(FrameBufferTextRenderingContext* ctx)
{
    FrameBuffer* fb = ctx->fb;

    void* buf = (void*)fb->vaddr;
    for (size_t i = 1; i < ctx->glyphs_per_col; ++i)
    {
        void* src = buf + i * 16 * fb->bytespp * fb->width;
        void* dest = buf + (i - 1) * 16 * fb->bytespp * fb->width;
        memcpy(dest, src, fb->width * fb->bytespp * 16);
    }

    /* Clear the last line. */
    memset(
        buf + (ctx->glyphs_per_col - 1) * 16 * fb->bytespp * fb->width,
        0,
        fb->width * fb->bytespp * 16);

    return 0;
}

static int fbsink_init(KOutputSink* sink)
{
    FrameBuffer* fb = fb_get_main();
    if (!fb)
        return -ENODEV;

    int st = psf_validate_builtin();
    if (st < 0)
    {
        KLOGF(WARN, "No PSF(U) file was linked into the kernel!");
        return st;
    }

    sink->fgcolor = KOUTPUT_WHITE;
    sink->bgcolor = KOUTPUT_BLACK;

    FrameBufferTextRenderingContext* ctx = sink->ctx;
    ctx->cx = 0;
    ctx->cy = 0;
    ctx->fb = fb;
    ctx->glyphs_per_col = fb->height / 16;
    ctx->glyphs_per_row = fb->width / 8;

    return 0;
}

static int fbsink_destroy(KOutputSink*)
{
    /* There's nothing to free. Only fb allocates stuff. */
    return 0;
}

static int fbsink_newline(KOutputSink* sink)
{
    /* Horrible hack, since dma pages are not mapped into user processes. I
     * don't know if, and *how* exactly we should do that... */
    Process* proc = sched_current_proc();
    if (proc)
        mm_load_kernel_paging_struct();

    FrameBufferTextRenderingContext* ctx = sink->ctx;

    ctx->cx = 0;

    if (ctx->cy + 1 >= ctx->glyphs_per_col)
        fbsink_scroll(ctx);
    else
        ++ctx->cy;

    if (proc)
        mm_load_paging_struct(&proc->paging_struct);

    return 0;
}

static int fbsink_print_char(char c, KOutputSink* sink)
{
    /* Horrible hack, since dma pages are not mapped into user processes. I
     * don't know if, and *how* exactly we should do that... */
    Process* proc = sched_current_proc();
    if (proc)
        mm_load_kernel_paging_struct();

    FrameBufferTextRenderingContext* ctx = sink->ctx;

    const PSF2Header* psfhdr = psf_get_builtin_header();

    u8 glyph_buf[16];
    psf_get_glyph_data(c, glyph_buf, psfhdr);

    size_t xoffset = ctx->cx * psfhdr->glyph_width;
    size_t yoffset = ctx->cy * psfhdr->glyph_height;

    u32 pixel = fbsink_koutput_color2pixel(sink->fgcolor);

    for (size_t i = 0; i < psfhdr->glyph_height; ++i)
    {
        u8 nib = glyph_buf[i];

        for (size_t k = 0; k < 8; ++k)
        {
            if (nib & (1 << k))
                fb_write_pixel(xoffset + (7 - k), yoffset + i, pixel);
        }
    }

    if (ctx->cx + 1 >= ctx->glyphs_per_row)
        fbsink_newline(sink);
    else
        ++ctx->cx;

    if (proc)
        mm_load_paging_struct(&proc->paging_struct);

    return 0;
}

static FrameBufferTextRenderingContext g_fb_ctx;

static KOutputSink g_fbsink = {
    .name = "framebuffer",
    .type = KOUTPUT_TERMINAL,
    .init = fbsink_init,
    .destroy = fbsink_destroy,
    .output_char = fbsink_print_char,
    .newline = fbsink_newline,
    .ctx = &g_fb_ctx};

static int fbsink_main()
{
    return kstdio_register_sink(&g_fbsink);
}

static int fbsink_exit()
{
    return kstdio_unregister_sink(&g_fbsink);
}

MODULE g_fbsink_module = {
    .name = "fbsink",
    .main = fbsink_main,
    .exit = fbsink_exit,
    .stage = MODULE_STAGE2};
