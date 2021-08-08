/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_X86_GDT_SYSGDT_H
#define _DXGMX_X86_GDT_SYSGDT_H

#define SYSGDT_KERNEL_CS 0x8
#define SYSGDT_KERNEL_DS 0x10
#define SYSGDT_USER_CS   0x18
#define SYSGDT_USER_DS   0x20

void sysgdt_init();

#endif //_DXGMX_X86_GDT_SYSGDT_H
