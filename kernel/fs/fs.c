/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/errno.h>
#include <dxgmx/fs/fs.h>
#include <dxgmx/fs/vnode.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/limits.h>
#include <dxgmx/string.h>
#include <dxgmx/todo.h>

VirtualNode* fs_new_vnode(FileSystem* fs, ino_t n, const VirtualNode* parent)
{
    // FIXME: remove this
    if (!fs || !n || (parent && !parent->n))
        return NULL;

    // FIXME: also remove tgis
    if (fs->vnodes)
    {
        FOR_EACH_ELEM_IN_DARR (fs->vnodes, fs->vnode_count, tmpvnode)
        {
            if (tmpvnode->n == n)
            {
                return NULL;
            }
        }
    }

    VirtualNode* tmp =
        krealloc(fs->vnodes, (fs->vnode_count + 1) * sizeof(VirtualNode));

    if (!tmp)
        return NULL;

    fs->vnodes = tmp;
    ++fs->vnode_count;

    VirtualNode* vnode = &fs->vnodes[fs->vnode_count - 1];
    memset(vnode, 0, sizeof(VirtualNode));

    vnode->owner = fs;
    vnode->n = n;
    vnode->parent_n = parent ? parent->n : 0;
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

int fs_make_path_relative(const FileSystem* fs, char* path)
{
    if (!path || !fs || !fs->mntpoint)
        return -EINVAL;

    size_t pathlen = strlen(path);
    size_t prefixlen = strlen(fs->mntpoint);

    if (prefixlen > pathlen)
        return -EINVAL;

    if (prefixlen == 1 && fs->mntpoint[0] == '/')
        return 0; /* That's it :) */

    size_t matching = 0;
    for (size_t i = 0; i < prefixlen; ++i)
        matching += (path[i] == fs->mntpoint[i]);

    if (matching != prefixlen)
        return -ENAMETOOLONG;

    for (size_t i = 0; i < prefixlen; ++i)
    {
        for (size_t k = 0; k < pathlen; ++k)
            path[k] = path[k + 1];
    }

    return 0;
}

int fs_make_path_canonical(char* path)
{
    (void)path;
    TODO_FATAL();
}

VirtualNode* fs_vnode_for_path(FileSystem* fs, const char* path)
{
    // FIXME: remove this
    if (!fs || !path)
        return NULL;

    char working_path[PATH_MAX];
    strncpy(working_path, path, PATH_MAX);

    fs_make_path_relative(fs, working_path);

    /* There are definitely better ways to do this, but for now this works
     * :)
     */
    VirtualNode* vnode_hit = NULL;
    FOR_EACH_ELEM_IN_DARR (fs->vnodes, fs->vnode_count, vnode)
    {
        VirtualNode* workingnode = vnode;
        char tmp[PATH_MAX] = {0};
        size_t running_len = 0;
        bool addslash = false;

        while (workingnode)
        {
            const size_t cur_len = strlen(workingnode->name) + addslash;
            if (running_len + cur_len >= PATH_MAX)
                break; // FIXME: errno ENAMETOOLONG

            for (size_t i = running_len + cur_len - 1; i >= cur_len; --i)
                tmp[i] = tmp[i - cur_len];

            memcpy(tmp, workingnode->name, cur_len - addslash);
            if (addslash)
                memcpy(tmp + cur_len - 1, "/", 1);

            running_len += cur_len;

            bool hit = false;
            FOR_EACH_ELEM_IN_DARR (fs->vnodes, fs->vnode_count, v)
            {
                if (v->n == workingnode->parent_n)
                {
                    workingnode = v;
                    hit = true;
                }
            }

            if (!hit)
                break;

            addslash = workingnode->parent_n != 0;
        }

        if (strcmp(tmp, working_path) == 0)
        {
            vnode_hit = vnode;
            break;
        }
    }

    return vnode_hit;
}
