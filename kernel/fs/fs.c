/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/fs/fs.h>
#include <dxgmx/fs/vnode.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/string.h>

struct S_VirtualNode* fs_new_vnode(FileSystem* fs)
{
    VirtualNode* tmp =
        krealloc(fs->vnodes, (fs->vnode_count + 1) * sizeof(VirtualNode));

    if (!tmp)
        return NULL;

    fs->vnodes = tmp;
    ++fs->vnode_count;

    VirtualNode* vnode = &fs->vnodes[fs->vnode_count - 1];
    memset(vnode, 0, sizeof(VirtualNode));
    return vnode;
}
