/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
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
    // FIXME: somewhere in here this code crashes if we have some driver expose
    // a vnode with it's name as an empty string. A driver shouldn't do this is
    // in the first place, but I feel like the problem runs a bit deeper with
    // this code.

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
            BREAK_LL(vnode);
        }
    }

    return vnode_hit;
}

VirtualNode* fs_new_vnode_cache(const char* name, FileSystem* fs)
{
    VirtualNode* new_vnode = kcalloc(sizeof(VirtualNode));
    if (!new_vnode)
        return NULL;

    new_vnode->name = strdup(name);
    new_vnode->owner = fs;
    new_vnode->ops = fs->driver->vnode_ops;
    if (!new_vnode->name)
    {
        kfree(new_vnode);
        return NULL;
    }

    if (linkedlist_add(new_vnode, &fs->vnode_ll) < 0)
    {
        kfree(new_vnode->name);
        kfree(new_vnode);
        return NULL;
    }

    return new_vnode;
}

int fs_free_cached_vnode(VirtualNode* vnode, FileSystem* fs)
{
    linkedlist_remove_by_data(vnode, &fs->vnode_ll);
    kfree(vnode->name);
    vnode->name = NULL;
    kfree(vnode);
    return 0;
}

int fs_free_all_cached_vnodes(FileSystem* fs)
{
    if (!fs->vnode_ll.root)
        return 0;

    do
    {
        VirtualNode* vnode = fs->vnode_ll.root->data;
        kfree(vnode->name);
        vnode->name = NULL;
        kfree(vnode);
    } while (linkedlist_remove_by_position(0, &fs->vnode_ll) == 0);

    return 0;
}

VirtualNode* fs_lookup_vnode(const char* path, FileSystem* fs)
{
    char* tmp = __builtin_alloca(strlen(path) + 1);
    strcpy(tmp, path);

    path_make_canonical(tmp);
    path_make_relative(tmp, fs);
    return fs_lookup_vnode_cached_relative(tmp, fs);
}

VirtualNode* fs_get_dir_vnode_for_file(const char* path, FileSystem* fs)
{
    const char* end = strrchr(path, '/');
    ASSERT(end); // we should at least have a leading /

    size_t newlen = end - path + 1;
    char* tmp = __builtin_alloca(newlen + 1);

    for (size_t i = 0; i < newlen; ++i)
        tmp[i] = path[i];

    if (tmp[newlen - 1] == '/' && newlen > 1)
        tmp[newlen - 1] = '\0';
    else
        tmp[newlen] = '\0';

    return fs_lookup_vnode_cached_relative(tmp, fs);
}

VirtualNode* fs_ino_to_vnode(ino_t ino, FileSystem* fs)
{
    FOR_EACH_ENTRY_IN_LL (fs->vnode_ll, VirtualNode*, vnode)
    {
        if (vnode->n == ino)
            return vnode;
    }

    return NULL;
}

ERR_OR(ino_t)
fs_mkfile(const char* path, mode_t mode, uid_t uid, gid_t gid, FileSystem* fs)
{
    char* tmp = __builtin_alloca(strlen(path) + 1);
    strcpy(tmp, path);

    path_make_canonical(tmp);
    path_make_relative(tmp, fs);
    VirtualNode* parent = fs_get_dir_vnode_for_file(tmp, fs);
    path_make_filename(tmp);

    ERR_OR(ino_t) res = fs->driver->mkfile(parent, tmp, mode, uid, gid, fs);
    return res;
}
