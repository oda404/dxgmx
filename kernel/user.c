/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/errno.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/user.h>

ERR_OR_PTR(char) user_strndup(const void* _USERPTR str, size_t maxn)
{
    ssize_t len = user_strnlen(str, maxn);
    if (len < 0)
        return ERR_PTR(char, len);

    if ((size_t)len == maxn)
        return ERR_PTR(char, -ENAMETOOLONG);

    void* dupstr = kmalloc(len + 1);
    if (!dupstr)
        return ERR_PTR(char, -ENOMEM);

    int st = user_copy_from(str, dupstr, len + 1);
    if (st < 0)
        return ERR_PTR(char, st);

    return VALUE_PTR(char, dupstr);
}
