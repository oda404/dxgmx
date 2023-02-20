/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/errno.h>
#include <dxgmx/klog.h>
#include <dxgmx/kstdio/sink.h>
#include <dxgmx/sched/sched.h>
#include <dxgmx/string.h>
#include <dxgmx/video/fb.h>
#include <dxgmx/video/psf.h>

#define KLOGF_PREFIX "fbtext: "

typedef struct S_FrameBufferTextRenderingContext
{
    FrameBuffer* fb;

    size_t cx;
    size_t cy;

    size_t glyphs_per_row;
    size_t glyphs_per_col;
} FrameBufferTextRenderingContext;

static u32 fbtext_koutput_color2pixel(KOutputColor color)
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

static int fbtext_scroll(FrameBufferTextRenderingContext* ctx)
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

static int fbtext_init(KOutputSink* sink)
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

static int fbtext_newline(KOutputSink* sink)
{
    /* Horrible hack, since dma pages are not mapped into user processes. I
     * don't know if, and *how* exactly we should do that... */
    Process* proc = sched_current_proc();
    if (proc)
        mm_load_kernel_paging_struct();

    FrameBufferTextRenderingContext* ctx = sink->ctx;

    ctx->cx = 0;

    if (ctx->cy + 1 >= ctx->glyphs_per_col)
        fbtext_scroll(ctx);
    else
        ++ctx->cy;

    if (proc)
        mm_load_paging_struct(&proc->paging_struct);

    return 0;
}

static int fbtext_print_char(char c, KOutputSink* sink)
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

    u32 pixel = fbtext_koutput_color2pixel(sink->fgcolor);

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
        fbtext_newline(sink);
    else
        ++ctx->cx;

    if (proc)
        mm_load_paging_struct(&proc->paging_struct);

    return 0;
}

static FrameBufferTextRenderingContext g_fb_ctx;

KOutputSink g_fb_koutput_sink = {
    .name = "framebuffer",
    .type = KOUTPUT_TERMINAL,
    .init = fbtext_init,
    .output_char = fbtext_print_char,
    .newline = fbtext_newline,
    .ctx = &g_fb_ctx};
