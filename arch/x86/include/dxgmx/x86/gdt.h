/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_X86_GDT_H
#define _DXGMX_X86_GDT_H

#ifndef _ASM

#include <dxgmx/types.h>

void gdt_finish_init();
void tss_set_esp0(ptr esp);
ptr tss_get_esp0();

#endif // !_ASM

#define GDT_ENTRIES_CNT 6

#define GDT_NULL 0x0
/* The ring 0 mode code segment */
#define GDT_KERNEL_CS 0x8
/* The ring 0 data segment. */
#define GDT_KERNEL_DS 0x10
/* The ring  3 code segment. */
#define GDT_USER_CS 0x18
/* The ring 3 data segment */
#define GDT_USER_DS 0x20
/* Tss */
#define GDT_TSS 0x28

#endif // _DXGMX_X86_GDT_H
