/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/errno.h>
#include <dxgmx/fs/path.h>
#include <dxgmx/math.h>
#include <dxgmx/string.h>

int path_make_relative(char* path, const FileSystem* fs)
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

int path_make_filename(char* path)
{
    char* end = strrchr(path, '/');
    size_t off = (size_t)((void*)end - (void*)path);
    strcpy(path, &path[off + 1]);
    return 0;
}

int path_make_canonical(char* path)
{
    /* FIXME: This function sucks ass, and can be done a lot better... it works
     * tho */

    size_t pathlen = strlen(path);

    // First pass to get rid of duplicate '/'s.
    {
        bool hit_del = false;
        for (size_t i = 0; i < pathlen; ++i)
        {
            char c = path[i];
            if (c == '/')
            {
                if (hit_del)
                {
                    for (size_t k = i; k < pathlen; ++k)
                        path[k] = path[k + 1];

                    --pathlen;
                    --i;
                }
                hit_del = true;
            }
            else
            {
                hit_del = false;
            }
        }
    }

    // Second pass to get rid of './' and '.'
    for (size_t i = 0; i < pathlen; ++i)
    {
        char c = path[i];
        if (c != '.')
            continue;

        if (i < pathlen - 1 && path[i + 1] == '/')
        {
            for (size_t k = i; k < pathlen - 1; ++k)
                path[k] = path[k + 2];

            pathlen -= 2;
        }
        else if (i == pathlen - 1)
        {
            path[pathlen-- - 1] = '\0';
        }
    }

    if (pathlen > 1 && path[pathlen - 1] == '/')
        path[pathlen - 1] = '\0';

    return 0;
}