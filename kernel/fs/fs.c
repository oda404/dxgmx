/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/errno.h>
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

int fs_rm_vnode(FileSystem* fs, struct S_VirtualNode* vnode)
{
    if (!fs || !vnode || !fs->vnodes)
        return -EINVAL;

    bool hit = false;
    FOR_EACH_ELEM_IN_DARR (fs->vnodes, fs->vnode_count, v)
    {
        if (v == vnode)
        {
            hit = true;
            break;
        }
    }

    if (!hit)
        return -ENOENT;

    if (vnode->name)
        kfree(vnode->name);

    for (VirtualNode* v = vnode; v < fs->vnodes + fs->vnode_count - 1; ++v)
        *v = *(v + 1);

    --fs->vnode_count;

    return 0;
}
