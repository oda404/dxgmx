/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/attrs.h>
#include <dxgmx/crypto/sha1.h>
#include <dxgmx/string.h>
#include <dxgmx/utils/bitwise.h>

/** Papa bless for this:
 * https://datatracker.ietf.org/doc/html/rfc3174
 */

typedef struct S_SHA1Context
{
    _ATTR_PACKED struct
    {
        u32 h0, h1, h2, h3, h4;
    } regs;
    struct
    {
        u32 buf[80];
        size_t size;
    } block;
    struct
    {
        u32 buf[16];
        bool has;
    } extrapadding;
    struct
    {
        const u8* buf;
        size_t initsize;
        size_t size;
    } msg;
} SHA1Context;

_ATTR_ALWAYS_INLINE static void sha1_pad(SHA1Context* ctx)
{
    ASSERT(ctx->block.size <= 64);

    size_t paddingsize = 64 - ctx->block.size;
    u64 msgbits = ctx->msg.initsize * 8;

    if (paddingsize >= 1 && paddingsize <= 8)
    {
        /*  Padding needs to be started from the original block,
        and continued onto an extra one.  */

        u8* block_buf_1byte = (u8*)ctx->block.buf;
        block_buf_1byte[ctx->block.size] = (1 << 7);
        memset(block_buf_1byte + ctx->block.size + 1, 0, paddingsize - 1);
        ctx->block.size = 64;

        /* Zero out everything but the last 16 bytes */
        memset(ctx->extrapadding.buf, 0, 48);
        /* Store the number of bits in the original message in the last
        16 bytes. */
        ctx->extrapadding.buf[14] =
            bw_u32_flip_endianness((msgbits >> 32) & 0xFFFFFFFF);
        ctx->extrapadding.buf[15] =
            bw_u32_flip_endianness(msgbits & 0xFFFFFFFF);
        ctx->extrapadding.has = true;
    }
    else
    {
        /* The only padding added is to the original block. */
        u8* block_buf_1byte = (u8*)ctx->block.buf;
        memset(block_buf_1byte + ctx->block.size, 0, paddingsize);
        /* Append 128 (0b10000000) after the message. */
        block_buf_1byte[ctx->block.size] = (1 << 7);

        /* Store the number of bits in the original message in the last
        16 bytes. */
        ctx->block.buf[14] =
            bw_u32_flip_endianness((msgbits >> 32) & 0xFFFFFFFF);
        ctx->block.buf[15] = bw_u32_flip_endianness(msgbits & 0xFFFFFFFF);
        ctx->block.size = 64;
    }
}

#define SHA1_PROCESS_BLOCK_SWP(a, b, c, d, e, f, k, t, block_buf)              \
    const u32 _tmp = bw_u32_rotl(a, 5) + f + e + block_buf[t] + k;             \
    e = d;                                                                     \
    d = c;                                                                     \
    c = bw_u32_rotl(b, 30);                                                    \
    b = a;                                                                     \
    a = _tmp;

static inline void
sha1_process_block_1(u32* a, u32* b, u32* c, u32* d, u32* e, u32* block_buf)
{
    u32 f;
    u32 k;

    for (u8 t = 0; t < 20; ++t)
    {
        f = ((*b) & (*c)) | ((~(*b)) & (*d));
        k = 0x5A827999;
        SHA1_PROCESS_BLOCK_SWP(*a, *b, *c, *d, *e, f, k, t, block_buf);
    }
}

static inline void
sha1_process_block_2(u32* a, u32* b, u32* c, u32* d, u32* e, u32* block_buf)
{
    u32 f;
    u32 k;

    for (u8 t = 20; t < 40; ++t)
    {
        f = (*b) ^ (*c) ^ (*d);
        k = 0x6ED9EBA1;
        SHA1_PROCESS_BLOCK_SWP(*a, *b, *c, *d, *e, f, k, t, block_buf);
    }
}

static inline void
sha1_process_block_3(u32* a, u32* b, u32* c, u32* d, u32* e, u32* block_buf)
{
    u32 f;
    u32 k;

    for (u8 t = 40; t < 60; ++t)
    {
        f = ((*b) & (*c)) | ((*b) & (*d)) | ((*c) & (*d));
        k = 0x8F1BBCDC;
        SHA1_PROCESS_BLOCK_SWP(*a, *b, *c, *d, *e, f, k, t, block_buf);
    }
}

static inline void
sha1_process_block_4(u32* a, u32* b, u32* c, u32* d, u32* e, u32* block_buf)
{
    u32 f;
    u32 k;

    for (u8 t = 60; t < 80; ++t)
    {
        f = (*b) ^ (*c) ^ (*d);
        k = 0xCA62C1D6;
        SHA1_PROCESS_BLOCK_SWP(*a, *b, *c, *d, *e, f, k, t, block_buf);
    }
}

static void sha1_process_block(SHA1Context* ctx)
{
    ASSERT(ctx->block.size == 64);

    u32 a = ctx->regs.h0;
    u32 b = ctx->regs.h1;
    u32 c = ctx->regs.h2;
    u32 d = ctx->regs.h3;
    u32 e = ctx->regs.h4;

    for (u8 i = 0; i < 16; ++i)
        ctx->block.buf[i] = bw_u32_flip_endianness(ctx->block.buf[i]);

    for (u8 i = 16; i < 80; ++i)
        ctx->block.buf[i] = bw_u32_rotl(
            ctx->block.buf[i - 3] ^ ctx->block.buf[i - 8] ^
                ctx->block.buf[i - 14] ^ ctx->block.buf[i - 16],
            1);

    sha1_process_block_1(&a, &b, &c, &d, &e, ctx->block.buf);
    sha1_process_block_2(&a, &b, &c, &d, &e, ctx->block.buf);
    sha1_process_block_3(&a, &b, &c, &d, &e, ctx->block.buf);
    sha1_process_block_4(&a, &b, &c, &d, &e, ctx->block.buf);

    ctx->regs.h0 += a;
    ctx->regs.h1 += b;
    ctx->regs.h2 += c;
    ctx->regs.h3 += d;
    ctx->regs.h4 += e;

    ctx->block.size = 0;

    /* Check if there is an extra block for padding, and process it. */
    if (ctx->extrapadding.has)
    {
        ctx->extrapadding.has = false;
        memcpy(ctx->block.buf, ctx->extrapadding.buf, 64);
        ctx->block.size = 64;
        sha1_process_block(ctx);
    }
}

static SHA1Context sha1_create_context(const char* msg, size_t msglen)
{
    return (SHA1Context){
        .regs.h0 = 0x67452301,
        .regs.h1 = 0xEFCDAB89,
        .regs.h2 = 0x98BADCFE,
        .regs.h3 = 0x10325476,
        .regs.h4 = 0xC3D2E1F0,
        .msg = {.buf = (const u8*)msg, .initsize = msglen, .size = msglen},
        .block = {.buf = {0}, .size = 0},
        .extrapadding = {.buf = {0}, .has = false}};
}

static size_t sha1_read_block(SHA1Context* ctx)
{
    size_t read = ctx->msg.size >= 64 ? 64 : ctx->msg.size;

    memcpy(ctx->block.buf, ctx->msg.buf, read);
    ctx->block.size = read;

    ctx->msg.buf += read;
    ctx->msg.size -= read;

    return read;
}

int sha1_chew(const char* buf, size_t buflen, u8* digest)
{
    SHA1Context ctx = sha1_create_context(buf, buflen);

    while (sha1_read_block(&ctx) == 64)
        sha1_process_block(&ctx);

    sha1_pad(&ctx);
    sha1_process_block(&ctx);

#ifdef CONFIG_LITTLE_ENDIAN
    ctx.regs.h0 = bw_u32_flip_endianness(ctx.regs.h0);
    ctx.regs.h1 = bw_u32_flip_endianness(ctx.regs.h1);
    ctx.regs.h2 = bw_u32_flip_endianness(ctx.regs.h2);
    ctx.regs.h3 = bw_u32_flip_endianness(ctx.regs.h3);
    ctx.regs.h4 = bw_u32_flip_endianness(ctx.regs.h4);
#endif

    memcpy(digest, &ctx.regs.h0, SHA1_DIGEST_SIZE);
    return 0;
}