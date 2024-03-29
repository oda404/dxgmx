
#include <dxgmx/devfs.h>
#include <dxgmx/errno.h>
#include <dxgmx/fs/vfs.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/limits.h>
#include <dxgmx/module.h>
#include <dxgmx/ramfs.h>
#include <dxgmx/stdio.h>
#include <dxgmx/string.h>

#define KLOGF_PREFIX "devfs: "

typedef struct DevFSEntry
{
    const char* name;
    mode_t mode;
    uid_t uid;
    gid_t gid;
    VirtualNodeOperations* ops;
    void* data;
} DevFSEntry;

static DevFSEntry* g_registered_entries;
static size_t g_registered_entry_count;

static int devfs_enlarge_entries()
{
    DevFSEntry* tmp = krealloc(
        g_registered_entries,
        (g_registered_entry_count + 1) * sizeof(DevFSEntry));

    if (!tmp)
        return -ENOMEM;

    ++g_registered_entry_count;
    g_registered_entries = tmp;
    return 0;
}

devfs_entry_t devfs_register(
    const char* name,
    mode_t mode,
    uid_t uid,
    gid_t gid,
    VirtualNodeOperations* ops,
    void* data)
{
    char* fullname = kmalloc(strlen(name) + 1 + name[0] != '/');
    if (!fullname)
        return -ENOMEM;

    if (name[0] != '/')
        strcpy(fullname, "/");
    strcat(fullname, name);

    if (devfs_enlarge_entries() < 0)
    {
        kfree(fullname);
        return -ENOMEM;
    }

    g_registered_entries[g_registered_entry_count - 1] = (DevFSEntry){
        .name = fullname,
        .mode = mode,
        .uid = uid,
        .gid = gid,
        .ops = ops,
        .data = data};

    return 0;
}

static int devfs_init(FileSystem* fs)
{
    int st = ramfs_init(fs);
    if (st < 0)
        return st;

    for (size_t i = 0; i < g_registered_entry_count; ++i)
    {
        DevFSEntry* entry = &g_registered_entries[i];

        size_t tmpstr_size = strlen(fs->mntpoint) + 1 + strlen(entry->name) + 1;
        if (tmpstr_size > PATH_MAX)
        {
            KLOGF(ERR, "Entry idx: %zu is bigger than PATH_MAX!", i);
            continue;
        }

        char* tmpstr = __builtin_alloca(tmpstr_size);
        sprintf(tmpstr, "%s/%s", fs->mntpoint, entry->name);

        ERR_OR(ino_t)
        res = fs_mkfile(tmpstr, entry->mode, entry->uid, entry->gid, fs);

        if (res.error < 0)
        {
            KLOGF(ERR, "Failed to create entry '%s'.", entry->name);
            continue;
        }

        VirtualNode* vnode = fs_ino_to_vnode(res.value, fs);
        vnode->ops = entry->ops;
        vnode->data = entry->data;
    }

    return 0;
}

static void devfs_destroy(FileSystem* fs)
{
    ramfs_destroy(fs);
    return;
}

#define MODULE_NAME "devfs"

static const VirtualNodeOperations g_devfs_vnode_ops = {
    .open = ramfs_open,
    .read = ramfs_read,
    .write = ramfs_write,
    .ioctl = ramfs_ioctl};

static const FileSystemDriver g_devfs_driver = {
    .name = MODULE_NAME,
    .generic_probe = false,
    .init = devfs_init,
    .destroy = devfs_destroy,
    .mkfile = ramfs_mkfile,
    .rmnode = ramfs_rmnode,
    .vnode_ops = &g_devfs_vnode_ops};

static int devfs_main()
{
    return vfs_register_fs_driver(&g_devfs_driver);
}

static int devfs_exit()
{
    return vfs_unregister_fs_driver(&g_devfs_driver);
}

MODULE g_devfs_module = {
    .name = MODULE_NAME, .main = devfs_main, .exit = devfs_exit};

#undef MODULE_NAME
