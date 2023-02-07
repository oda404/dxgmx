/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/errno.h>
#include <dxgmx/fs/fs.h>
#include <dxgmx/fs/path.h>
#include <dxgmx/fs/vnode.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/limits.h>
#include <dxgmx/string.h>
#include <dxgmx/todo.h>

/**
 * Lookup cached vnode for 'path' that is residing on 'fs'. If this function
 * returns NULL it means there is no *cached* vnode for this path. The
 * filesystem might have this vnode, but has not yet cached it. For situations
 * like these try fs_lookup_vnode, which first tries fs_lookup_vnode_cached.
 *
 * No null pointers should be passed to this function.
 *
 * 'path' A relative to the filesystem path.
 * 'fs' The filesystem to scan.
 *
 * Returns:
 * A VirtualNode* on success.
 * NULL if no cached vnode has been found.
 */
static VirtualNode*
fs_lookup_vnode_cached_relative(const char* path, FileSystem* fs)
{
    VirtualNode* vnode_hit = NULL;
    FOR_EACH_ENTRY_IN_LL (fs->vnode_ll, VirtualNode*, vnode)
    {
        char tmp_path[PATH_MAX] = {0};

        size_t running_len = 0;
        bool addslash = false;

        const VirtualNode* working_vnode = vnode;
        while (working_vnode)
        {
            const size_t cur_len = strlen(working_vnode->name) + addslash;
            if (running_len + cur_len >= PATH_MAX)
                break;

            for (size_t i = running_len + cur_len - 1; i >= cur_len; --i)
                tmp_path[i] = tmp_path[i - cur_len];

            memcpy(tmp_path, working_vnode->name, cur_len - addslash);
            if (addslash)
                memcpy(tmp_path + cur_len - 1, "/", 1);

            running_len += cur_len;

            /* We hit the root dir */
            if (!working_vnode->parent)
                break;

            working_vnode = working_vnode->parent;

            addslash = working_vnode->parent != 0;
        }

        if (strcmp(tmp_path, path) == 0)
        {
            vnode_hit = vnode;
            break;
        }
    }

    return vnode_hit;
}

VirtualNode* fs_new_vnode_cache(FileSystem* fs)
{
    VirtualNode* new_vnode = kcalloc(sizeof(VirtualNode));
    if (!new_vnode)
        return NULL;

    new_vnode->owner = fs;
    new_vnode->ops = fs->driver->vnode_ops;

    if (linkedlist_add(new_vnode, &fs->vnode_ll) < 0)
    {
        kfree(new_vnode);
        return NULL;
    }

    return new_vnode;
}

int fs_rm_cached_vnode(VirtualNode* vnode, FileSystem* fs)
{
    int st = linkedlist_remove_by_data(vnode, &fs->vnode_ll);
    if (st == 0)
        kfree(vnode);

    return st;
}

VirtualNode* fs_lookup_vnode(const char* path, FileSystem* fs)
{
    char working_path[PATH_MAX];
    strncpy(working_path, path, PATH_MAX);

    path_make_relative(fs, working_path);

    /* Look for any cached vnodes */
    VirtualNode* vnode = fs_lookup_vnode_cached_relative(working_path, fs);
    if (!vnode)
    {
        /* That failed, so we query the driver */
        vnode = fs->driver->vnode_lookup(working_path, fs);
    }

    return vnode;
}
