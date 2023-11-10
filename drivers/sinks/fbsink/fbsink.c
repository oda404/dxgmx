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
#include <dxgmx/proc/procm.h>
#include <dxgmx/string.h>

#define KLOGF_PREFIX "fbsink: "

typedef struct FrameBufferRenderContext
{
    FrameBuffer* fb;
    size_t cx;
    size_t cy;
    size_t glyphs_per_row;
    size_t glyphs_per_col;
} FrameBufferRenderContext;

typedef struct S_FrameBufferSinkContext
{
    FrameBufferPack* fbpack;

    FrameBufferRenderContext* contexts;
    size_t context_count;

    const PSF2Header* font;
    void* glyph_buf;
} FrameBufferSinkContext;

static int fbsink_validate_render_contexts(FrameBufferSinkContext* ctx)
{
    (void)ctx;
    return 0;
}

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

static int
fbsink_scroll(KOutputSink* sink, FrameBufferRenderContext* render_ctx)
{
    FrameBuffer* fb = render_ctx->fb;
    const PSF2Header* font = ((FrameBufferSinkContext*)sink->ctx)->font;

    const size_t rowsize = fb->width * fb->bytespp * font->glyph_height;
    const size_t rowoffset = font->glyph_height * fb->bytespp * fb->width;
    void* buf = (void*)fb->base_va;

    for (size_t i = 1; i < render_ctx->glyphs_per_col; ++i)
    {
        void* src = buf + i * rowoffset;
        void* dst = buf + (i - 1) * rowoffset;
        memcpy(dst, src, rowsize);
    }

    /* Clear the last line. */
    size_t off = (render_ctx->glyphs_per_col - 1) * rowoffset;
    memset(buf + off, 0, rowsize);
    return 0;
}

static int fbsink_init(KOutputSink* sink)
{
    FrameBufferPack* fbpack = fb_get_pack();
    if (*fbpack->fb_count == 0)
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

    sink->ctx = kmalloc(sizeof(FrameBufferSinkContext));
    if (!sink->ctx)
        return -ENOMEM;

    FrameBufferSinkContext* ctx = sink->ctx;
    ctx->glyph_buf = kmalloc(font->glyph_size);
    ctx->font = font;
    if (!ctx->glyph_buf)
    {
        ctx->font = NULL;
        kfree(sink->ctx);
        sink->ctx = NULL;
        return -ENOMEM;
    }

    ctx->contexts =
        kmalloc(*fbpack->fb_count * sizeof(FrameBufferRenderContext));
    if (!ctx->contexts)
    {
        ctx->font = NULL;
        kfree(ctx->glyph_buf);
        ctx->glyph_buf = NULL;
        kfree(sink->ctx);
        sink->ctx = NULL;
        return -ENOMEM;
    }

    for (size_t i = 0; i < *fbpack->fb_count; ++i)
    {
        FrameBuffer* fb = &(*fbpack->fbs)[i];
        ctx->contexts[i].cx = 0;
        ctx->contexts[i].cy = 0;
        ctx->contexts[i].fb = fb;
        ctx->contexts[i].glyphs_per_col = fb->height / font->glyph_height;
        ctx->contexts[i].glyphs_per_row = fb->width / font->glyph_width;
    }

    ctx->context_count = *fbpack->fb_count;
    ctx->fbpack = fbpack;
    return 0;
}

static int fbsink_destroy(KOutputSink* sink)
{
    FrameBufferSinkContext* ctx = sink->ctx;
    kfree(ctx->contexts);
    ctx->contexts = NULL;
    ctx->context_count = 0;
    ctx->fbpack = NULL;
    ctx->font = NULL;
    kfree(ctx->glyph_buf);
    ctx->glyph_buf = NULL;
    kfree(sink->ctx);
    sink->ctx = NULL;
    return 0;
}

static int
fbsink_newline_to_ctx(KOutputSink* sink, FrameBufferRenderContext* render_ctx)
{
    render_ctx->cx = 0;
    if (render_ctx->cy + 1 >= render_ctx->glyphs_per_col)
        fbsink_scroll(sink, render_ctx);
    else
        ++render_ctx->cy;

    return 0;
}

static int fbsink_newline(KOutputSink* sink)
{
    FrameBufferSinkContext* ctx = sink->ctx;
    fbsink_validate_render_contexts(ctx);

    for (size_t i = 0; i < ctx->context_count; ++i)
    {
        FrameBufferRenderContext* render_ctx = &ctx->contexts[i];
        if (render_ctx->fb->takeover)
            continue;

        fbsink_newline_to_ctx(sink, render_ctx);
    }
    return 0;
}

static int fbsink_print_char_to_ctx(
    char c, KOutputSink* sink, FrameBufferRenderContext* render_ctx)
{
    FrameBuffer* fb = render_ctx->fb;
    FrameBufferSinkContext* ctx = sink->ctx;
    const PSF2Header* font = ctx->font;

    const size_t x = render_ctx->cx * font->glyph_width;
    const size_t y = render_ctx->cy * font->glyph_height;
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
                fb_write_pixel(
                    x + (font->glyph_width - 1 - k), y + i, pixel, fb);
        }
    }

    if (render_ctx->cx + 1 >= render_ctx->glyphs_per_row)
        fbsink_newline_to_ctx(sink, render_ctx);
    else
        ++render_ctx->cx;

    return 0;
}

static int fbsink_print_char(char c, KOutputSink* sink)
{
    FrameBufferSinkContext* ctx = sink->ctx;
    fbsink_validate_render_contexts(ctx);

    for (size_t i = 0; i < ctx->context_count; ++i)
    {
        FrameBufferRenderContext* render_ctx = &ctx->contexts[i];
        if (render_ctx->fb->takeover)
            continue;

        fbsink_print_char_to_ctx(c, sink, render_ctx);
    }
    return 0;
}

static KOutputSink g_fbsink = {
    .name = "framebuffer",
    .type = KOUTPUT_TERMINAL,
    .init = fbsink_init,
    .destroy = fbsink_destroy,
    .output_char = fbsink_print_char,
    .newline = fbsink_newline};

static int fbsink_main()
{
    int st = mod_builtin_depends_on("fb");
    if (st < 0)
        return st;

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
