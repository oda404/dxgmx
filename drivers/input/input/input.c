/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/input/input.h>

devfs_entry_t input_register(
    const char* name,
    mode_t mode,
    uid_t uid,
    gid_t gid,
    VirtualNodeOperations* vops,
    void* data)
{
    // FIXME: should be under /dev/input/
    return devfs_register(name, mode, uid, gid, vops, data);
}
