
#include<dxgmx/vfs.h>
#include<dxgmx/kmalloc.h>
#include<dxgmx/panic.h>
#include<dxgmx/klog.h>
#include<dxgmx/string.h>

#define KLOGF(lvl, fmt, ...) klogln(lvl, "vfs: " fmt, ##__VA_ARGS__)

static GenericDrive *g_drives = NULL;
static size_t g_drives_count = 0;

bool vfs_add_drive(const GenericDrive *d)
{
    if(!d)
        return false;

    g_drives = krealloc(g_drives, (++g_drives_count) * sizeof(GenericDrive));
    if(!g_drives)
        panic("Falied to allocate VFS device!");

    GenericDrive *drive = &g_drives[g_drives_count - 1];
    /* We memcpy since there are some const members that we can't assign. */
    memcpy(drive, d, sizeof(GenericDrive));

    drive->uid = 0;
    drive->partitions_count = 0;
    drive->partitions = NULL;

    KLOGF(INFO, "Registered drive: %s.", drive->name);

    return true;
}

bool vfs_remove_drive()
{
    return false;
}

bool vfs_init()
{
    return false;
}

bool vfs_mount(const char *name, const char *mountpoint)
{
    return false;
}

bool vfs_umount(const char *name_or_mountpoint)
{
    return false;
}
