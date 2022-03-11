/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_ERRNO_H
#define _DXGMX_ERRNO_H

#define ERANGE 1
#define EINVAL 2

int* _get_errno_addr();
/*
 * errno is the one thing that is not prefixed with k
 * because of later libc usage
 */
#define errno (*_get_errno_addr())

#endif // _DXGMX_ERRNO_H
