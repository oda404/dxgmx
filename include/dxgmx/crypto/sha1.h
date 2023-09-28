/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_CRYPTO_SHA1_H
#define _DXGMX_CRYPTO_SHA1_H

#include <dxgmx/types.h>

#define SHA1_DIGEST_SIZE 20

int sha1_chew(const char* buf, size_t buflen, u8* digest);

#endif //!_DXGMX_CRYPTO_SHA1_H