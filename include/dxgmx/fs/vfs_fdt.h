
#ifndef _DXGMX_FS_VFS_FDT_H
#define _DXGMX_FS_VFS_FDT_H

/* The system wide open File Descriptor Table (FDT) */

#include <dxgmx/fs/fd.h>
#include <dxgmx/proc/proc.h>

int vfs_fdt_init();

FileDescriptor* vfs_fdt_new_sysfd(fd_t procfd, pid_t pid);
FileDescriptor* vfs_fdt_get_sysfd(fd_t procd, pid_t pid);
void vfs_fdt_free_sysfd(fd_t procfd, pid_t pid);

#endif // !_DXGMX_FS_VFS_FDT_H
