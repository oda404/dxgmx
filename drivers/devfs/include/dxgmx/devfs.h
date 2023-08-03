
#ifndef _DXGMX_DEVFS_H
#define _DXGMX_DEVFS_H

#include <dxgmx/fs/vfs.h>
#include <dxgmx/types.h>
#include <posix/sys/types.h>

typedef int devfs_entry_t;

devfs_entry_t devfs_register(
    const char* name,
    mode_t mode,
    uid_t uid,
    gid_t gid,
    VirtualNodeOperations* ops);

#endif // !_DXGMX_DEVFS_H