/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_ELF_ELFLOADER_H
#define _DXGMX_ELF_ELFLOADER_H

#include <dxgmx/proc/proc.h>

/**
 * Load an ELF executable into memory, mapping any necessary pages, and setting
 * the instruction pointer. This function assumes the file was already validated
 * using elfloader_validate_file().
 *
 * No NULL pointers should be passed to this function.
 * No invalid file descriptors should be passed to this function.
 *
 * 'fd' Open fd of the file.
 * 'actingproc' Acting process.
 * 'targetproc' The process for which we are loading the ELF.
 *
 * Returns:
 * 0 if valid.
 * -EINVAL on invalid ELF.
 */
int elfloader_load_from_file(fd_t fd, Process* actingproc, Process* targetprot);

#endif // !_DXGMX_ELF_ELFLOADER_H
