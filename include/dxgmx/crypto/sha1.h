/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_CRYPTO_SHA1_H
#define _DXGMX_CRYPTO_SHA1_H

#include<dxgmx/types.h>

typedef struct S_SHA1Digest
{
    char hash[41];
    size_t hashlen;
} SHA1Digest;

SHA1Digest sha1_hash(const char *str);
SHA1Digest sha1_hash_buf(const char *buf, size_t buflen);

#endif //!_DXGMX_CRYPTO_SHA1_H
