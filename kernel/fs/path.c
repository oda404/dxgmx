/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/errno.h>
#include <dxgmx/fs/path.h>
#include <dxgmx/string.h>

int path_make_relative(const FileSystem* fs, char* path)
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
