/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_KIMG_H
#define _DXGMX_KIMG_H

#include <dxgmx/types.h>

/* Information about the kernel image / sections. */

ptr kimg_bootloader_start();
ptr kimg_bootloader_end();

ptr kimg_text_start();
ptr kimg_text_end();

ptr kimg_stack_top();
ptr kimg_stack_bot();

ptr kimg_bss_start();
ptr kimg_bss_end();

ptr kimg_data_start();
ptr kimg_data_end();

ptr kimg_rodata_start();
ptr kimg_rodata_end();

ptr kimg_ro_postinit_start();
ptr kimg_ro_postinit_end();

ptr kimg_init_start();
ptr kimg_init_end();

ptr kimg_kinit_stage3_text_start();
ptr kimg_kinit_stage3_text_end();

ptr kimg_module_start();
ptr kimg_module_end();

ptr kimg_ksyms_start();
ptr kimg_ksyms_end();

/* The higher half map offset. */
size_t kimg_map_offset();

/* Physical address of the kernel image. */
ptr kimg_paddr();
/* Virtual address of the kernel image. */
ptr kimg_vaddr();
/* Size of the kernel image. */
size_t kimg_size();

#endif // !_DXGMX_KIMG_H
