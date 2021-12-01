/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/crypto/sha1.h>
#include<dxgmx/string.h>
#include<dxgmx/types.h>
#include<dxgmx/assert.h>
#include<dxgmx/utils/bitwise.h>
#include<dxgmx/stdlib.h>
#include<dxgmx/stdio.h>

/** Papa bless for this:
 * https://datatracker.ietf.org/doc/html/rfc3174
*/

typedef struct S_SHA1Context
{
    struct
    {
        u32 h0, h1, h2, h3, h4;
    } regs;
    struct
    {
        u8 buf[64];
        size_t size;
    } block;
    struct
    {
        u8 buf[64];
        bool has;
    } extrapadding;
    struct
    {
        const u8 *buf;
        size_t initsize;
        size_t size;
    } msg;
} SHA1Context;

static u32 
sha1_w(const u32 buf[16], size_t t)
{
    return t < 16 ? 
        bw_u32_flip_endianness(buf[t]) :
        bw_u32_rotl(sha1_w(buf, t - 3) ^ sha1_w(buf, t - 8) ^ sha1_w(buf, t - 14) ^ sha1_w(buf, t - 16), 1);
}

_ATTR_ALWAYS_INLINE static void 
sha1_pad(SHA1Context *ctx)
{
    ASSERT(ctx->block.size <= 64);

    size_t paddingsize = 64 - ctx->block.size;
    u64 bitsinmsg = ctx->msg.initsize * 8;
    
    switch(paddingsize)
    {
    case 1 ... 8: 
        /*  Padding needs to be started from the original block,
        and continued onto an extra one.  */
        ctx->block.buf[ctx->block.size] = (1 << 7);
        memset(ctx->block.buf + ctx->block.size + 1, 0, paddingsize - 1);
        ctx->block.size = 64;
        
        /* Zero out everything but the last 16 bytes */
        memset(ctx->extrapadding.buf, 0, 48);
        /* Store the number of bits in the original message in the last
        16 bytes. */
        ((u32*)ctx->extrapadding.buf)[14] = bw_u32_flip_endianness((bitsinmsg >> 32) & 0xFFFFFFFF);
        ((u32*)ctx->extrapadding.buf)[15] = bw_u32_flip_endianness(bitsinmsg & 0xFFFFFFFF);
        ctx->extrapadding.has = true;
        break;

    case 0:
        /* No padding is added to the original block, but an extra
        one is needed. */
        /* Zero out padding  */
        memset(ctx->extrapadding.buf + 1, 0, 47);
        ctx->extrapadding.buf[0] = (1 << 7);
        /* Store the number of bits in the original message in the last
        16 bytes. */
        ((u32*)ctx->extrapadding.buf)[14] = bw_u32_flip_endianness((bitsinmsg >> 32) & 0xFFFFFFFF);
        ((u32*)ctx->extrapadding.buf)[15] = bw_u32_flip_endianness(bitsinmsg & 0xFFFFFFFF);
        ctx->extrapadding.has = true;
        break;

    default:
        /* The only padding added is to the original block. */
        memset(ctx->block.buf + ctx->block.size, 0, paddingsize - 16);
        /* Append 128 (0b10000000) after the message. */
        ctx->block.buf[ctx->block.size] = (1 << 7);
        /* Store the number of bits in the original message in the last
        16 bytes. */
        ((u32*)ctx->block.buf)[14] = bw_u32_flip_endianness((bitsinmsg >> 32) & 0xFFFFFFFF);
        ((u32*)ctx->block.buf)[15] = bw_u32_flip_endianness(bitsinmsg & 0xFFFFFFFF);
        ctx->block.size = 64;
        break;
    }
}

static void sha1_process_block(SHA1Context *ctx)
{
    ASSERT(ctx->block.size == 64)

    u32 a = ctx->regs.h0;
    u32 b = ctx->regs.h1;
    u32 c = ctx->regs.h2;
    u32 d = ctx->regs.h3;
    u32 e = ctx->regs.h4;

    for(size_t t = 0; t < 80; ++t)
    {
        u32 f;
        u32 k;

        switch(t)
        {
        case 0 ... 19:
            f = (b & c) | ((~b) & d);
            k = 0x5A827999;
            break;

        case 20 ... 39:
            f = b ^ c ^ d;
            k = 0x6ED9EBA1;
            break;

        case 40 ... 59:
            f = (b & c) | (b & d) | (c & d);
            k = 0x8F1BBCDC;
            break;

        case 60 ... 79:
            f = b ^ c ^ d;
            k = 0xCA62C1D6;
            break;
        }

        u32 tmp = bw_u32_rotl(a, 5) + f + e + sha1_w((u32*)ctx->block.buf, t) + k;

        e = d;
        d = c;
        c = bw_u32_rotl(b, 30);
        b = a;
        a = tmp;
    }

    ctx->regs.h0 += a;
    ctx->regs.h1 += b;
    ctx->regs.h2 += c;
    ctx->regs.h3 += d;
    ctx->regs.h4 += e;

    ctx->block.size = 0;

    /* Check if there is an extra block for padding, and process it. */
    if(ctx->extrapadding.has)
    {
        ctx->extrapadding.has = false;
        memcpy(ctx->block.buf, ctx->extrapadding.buf, 64);
        ctx->block.size = 64;
        sha1_process_block(ctx);
    }
}

static SHA1Context sha1_create_context(const char *msg, size_t msglen)
{
    return (SHA1Context){
        .regs.h0 = 0x67452301,
        .regs.h1 = 0xEFCDAB89,
        .regs.h2 = 0x98BADCFE,
        .regs.h3 = 0x10325476,
        .regs.h4 = 0xC3D2E1F0,
        .msg = {
            .buf = (const u8*)msg,
            .initsize = msglen,
            .size = msglen
        },
        .block = {
            .buf = { 0 },
            .size = 0
        },
        .extrapadding = {
            .buf = { 0 },
            .has = false
        }
    };
}

static size_t sha1_read_block(SHA1Context *ctx)
{
    size_t read = ctx->msg.size >= 64 ? 64 : ctx->msg.size;

    memcpy(ctx->block.buf, ctx->msg.buf, read);
    ctx->block.size = read;

    ctx->msg.buf += read;
    ctx->msg.size -= read;

    return read;
}

SHA1Digest sha1_hash(const char *str)
{
    return sha1_hash_buf(str, strlen(str));
}

SHA1Digest sha1_hash_buf(const char *buf, size_t buflen)
{
    SHA1Context ctx = sha1_create_context(buf, buflen);

    while(sha1_read_block(&ctx) == 64)
        sha1_process_block(&ctx);

    sha1_pad(&ctx);
    sha1_process_block(&ctx);

    SHA1Digest digest = {
        .hashsize = 20,
        .h0 = ctx.regs.h0,
        .h1 = ctx.regs.h1,
        .h2 = ctx.regs.h2,
        .h3 = ctx.regs.h3,
        .h4 = ctx.regs.h4,
    };

    return digest;
}
