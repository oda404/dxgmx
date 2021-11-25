/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_CRYPTO_SHA1_H
#define _DXGMX_CRYPTO_SHA1_H

#include<dxgmx/types.h>

typedef struct S_SHA1Digest
{
    union
    {
        struct
        {
            u32 h0;
            u32 h1;
            u32 h2;
            u32 h3;
            u32 h4;
        };
        u32 hashbuf[5];
    };
    
    /* in bytes */
    size_t hashsize;
} SHA1Digest;

SHA1Digest sha1_hash(const char *str);
SHA1Digest sha1_hash_buf(const char *buf, size_t buflen);

#endif //!_DXGMX_CRYPTO_SHA1_H
