/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include "psf.h"
#include <dxgmx/errno.h>
#include <dxgmx/fb.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/kstdio/kstdio.h>
#include <dxgmx/kstdio/sink.h>
#include <dxgmx/module.h>
#include <dxgmx/sched/sched.h>
#include <dxgmx/string.h>

#define KLOGF_PREFIX "fbsink: "

typedef struct S_FrameBufferTextRenderingContext
{
    FrameBuffer* fb;

    size_t cx;
    size_t cy;

    size_t glyphs_per_row;
    size_t glyphs_per_col;

    const PSF2Header* font;
    void* glyph_buf;
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
    const PSF2Header* font = ctx->font;

    const size_t rowsize = fb->width * fb->bytespp * font->glyph_height;
    const size_t rowoffset = font->glyph_height * fb->bytespp * fb->width;
    void* buf = (void*)fb->vaddr;

    for (size_t i = 1; i < ctx->glyphs_per_col; ++i)
    {
        void* src = buf + i * rowoffset;
        void* dst = buf + (i - 1) * rowoffset;
        memcpy(dst, src, rowsize);
    }

    /* Clear the last line. */
    size_t off = (ctx->glyphs_per_col - 1) * rowoffset;
    memset(buf + off, 0, rowsize);

    return 0;
}

static int fbsink_init(KOutputSink* sink)
{
    if (fb_ensure_init() < 0)
        return -ENODEV;

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

    const PSF2Header* font = psf_get_builtin_header();
    if (font->glyph_width >= 32)
        return -EINVAL; // goofy ahh font

    FrameBufferTextRenderingContext* ctx = sink->ctx;
    ctx->glyph_buf = kmalloc(font->glyph_size);
    if (!ctx->glyph_buf)
        return -ENOMEM;

    ctx->cx = 0;
    ctx->cy = 0;
    ctx->fb = fb;
    ctx->glyphs_per_col = fb->height / font->glyph_height;
    ctx->glyphs_per_row = fb->width / font->glyph_width;
    ctx->font = font;

    return 0;
}

static int fbsink_destroy(KOutputSink* sink)
{
    FrameBufferTextRenderingContext* ctx = sink->ctx;
    kfree(ctx->glyph_buf);
    return 0;
}

static int fbsink_newline(KOutputSink* sink)
{
    FrameBufferTextRenderingContext* ctx = sink->ctx;
    ctx->cx = 0;

    if (ctx->cy + 1 >= ctx->glyphs_per_col)
        fbsink_scroll(ctx);
    else
        ++ctx->cy;

    return 0;
}

static int fbsink_print_char(char c, KOutputSink* sink)
{
    FrameBufferTextRenderingContext* ctx = sink->ctx;
    const PSF2Header* font = ctx->font;

    const size_t x = ctx->cx * font->glyph_width;
    const size_t y = ctx->cy * font->glyph_height;
    const size_t bytes_per_row = font->glyph_width / 8;
    const u32 pixel = fbsink_koutput_color2pixel(sink->fgcolor);
    u32 glyphrow;

    psf_get_glyph_data(c, ctx->glyph_buf, font);

    for (size_t i = 0; i < font->glyph_height; ++i)
    {
        memcpy(&glyphrow, ctx->glyph_buf + i * bytes_per_row, bytes_per_row);

        for (size_t k = 0; k < font->glyph_width; ++k)
        {
            if (glyphrow & (1 << k))
                fb_write_pixel(x + (font->glyph_width - 1 - k), y + i, pixel);
        }
    }

    if (ctx->cx + 1 >= ctx->glyphs_per_row)
        fbsink_newline(sink);
    else
        ++ctx->cx;

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
