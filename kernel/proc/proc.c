/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/errno.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/proc/proc.h>
#include <dxgmx/todo.h>

int proc_new_fd(size_t sysfd_idx, Process* proc)
{
    if (sysfd_idx == PLATFORM_MAX_UNSIGNED)
        return -E2BIG;

    /* Try to find the first available fd */
    for (size_t i = 0; i < proc->fd_count; ++i)
    {
        if (proc->fds[i] == PLATFORM_MAX_UNSIGNED)
        {
            proc->fds[i] = sysfd_idx;
            return i;
        }
    }

    /* No free fd was found, allocate new one */
    size_t* tmp = krealloc(proc->fds, (proc->fd_count + 1) * sizeof(size_t));
    if (!tmp)
        return -ENOMEM;

    proc->fds = tmp;
    ++proc->fd_count;

    int fd = proc->fd_count - 1;
    proc->fds[fd] = sysfd_idx;
    return fd;
}

size_t proc_free_fd(fd_t fd, Process* proc)
{
    if (fd < 0 || (size_t)fd >= proc->fd_count)
        return PLATFORM_MAX_UNSIGNED;

    /* FIXME: shrink array once we a hit a threshold. */

    size_t ret = proc->fds[fd];
    proc->fds[fd] = PLATFORM_MAX_UNSIGNED; // available flag
    return ret;
}
