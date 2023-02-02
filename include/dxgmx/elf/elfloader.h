/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_ELF_ELFLOADER_H
#define _DXGMX_ELF_ELFLOADER_H

#include <dxgmx/proc/proc.h>
#include <dxgmx/types.h>

/**
 * Check if  a file is a valid ELF executable.
 *
 * No NULL pointers should be passed to this function.
 * No invalid file descriptors should be passed to this function.
 *
 * 'fd' Open fd of the file.
 * 'proc' Acting process.
 *
 * Returns:
 * 0 if valid.
 * -EINVAL if not a ELF file.
 * -ENOEXEC if not an executable ELF.
 */
int elfloader_validate_file(fd_t fd, Process* actingproc);

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
