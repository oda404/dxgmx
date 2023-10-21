/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/fs/vfs_fdt.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/string.h>
#include <dxgmx/utils/hashtable.h>

static HashTable g_sys_fd_hashtable;

static hashtable_key_t vfs_fdt_combo_to_hashkey(fd_t procfd, pid_t pid)
{
    return (((hashtable_key_t)procfd) << 32) | (u32)pid;
}

int vfs_fdt_init()
{
    return hashtable_init(&g_sys_fd_hashtable);
}

FileDescriptor* vfs_fdt_new_sysfd(fd_t procfd, pid_t pid)
{
    FileDescriptor* sysfd = kcalloc(sizeof(FileDescriptor));
    if (!sysfd)
        return NULL;

    int st = hashtable_insert(
        vfs_fdt_combo_to_hashkey(procfd, pid),
        (hashtable_val_t)sysfd,
        &g_sys_fd_hashtable);

    if (st < 0)
    {
        kfree(sysfd);
        return NULL;
    }

    sysfd->pid = pid;
    sysfd->fd = procfd;
    return sysfd;
}

FileDescriptor* vfs_fdt_get_sysfd(fd_t procfd, pid_t pid)
{
    ERR_OR(hashtable_val_t)
    find_res = hashtable_find(
        vfs_fdt_combo_to_hashkey(procfd, pid), &g_sys_fd_hashtable);

    if (find_res.error)
        return NULL;

    return (FileDescriptor*)find_res.value;
}

void vfs_fdt_free_sysfd(FileDescriptor* sysfd)
{
    hashtable_remove(
        vfs_fdt_combo_to_hashkey(sysfd->fd, sysfd->pid), &g_sys_fd_hashtable);
    kfree(sysfd);
}
