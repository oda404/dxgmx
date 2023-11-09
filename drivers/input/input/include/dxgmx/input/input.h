
#ifndef _DXGMX_DRIVERS_INPUT_INPUT_INPUT_H
#define _DXGMX_DRIVERS_INPUT_INPUT_INPUT_H

#include <dxgmx/devfs.h>
#include <dxgmx/input/user@input.h>

devfs_entry_t input_register(
    const char* name,
    mode_t mode,
    uid_t uid,
    gid_t gid,
    VirtualNodeOperations* vops,
    void* data);

#endif // !_DXGMX_DRIVERS_INPUT_INPUT_INPUT_H
