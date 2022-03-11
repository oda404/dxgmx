/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_KSTDIO_H
#define _DXGMX_KSTDIO_H

#include <dxgmx/types.h>

bool kstdio_init();
size_t kstdio_write(const char* buf, size_t n);
void kstdio_set_serial_debug(bool enabled);

#endif // !_DXGMX_KSTDIO_H
