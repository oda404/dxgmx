/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_KSTDIO_H
#define _DXGMX_KSTDIO_H

#include <dxgmx/kstdio/sink.h>
#include <dxgmx/types.h>

int kstdio_register_sink(KOutputSink* sink);
int kstdio_unregister_sink(KOutputSink* sink);

size_t kstdio_write(const char* buf, size_t n);

size_t kstdio_write_raw(const char* buf, size_t n);

#endif // !_DXGMX_KSTDIO_H
