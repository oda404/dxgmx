/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/panic.h>
#include <dxgmx/storage/mbr.h>
#include <dxgmx/string.h>
#include <dxgmx/vfs.h>

#define KLOGF(lvl, fmt, ...) klogln(lvl, "vfs: " fmt, ##__VA_ARGS__)

static GenericDrive* g_drives = NULL;
static size_t g_drives_count = 0;

static void vfs_dump_drive_info(const GenericDrive* drive)
{
    KLOGF(
        INFO,
        "/dev/%s: (UID 0x%X) with %u sector size",
        drive->name,
        (u32)drive->uid,
        drive->sectorsize);

    for (size_t i = 0; i < drive->partitions_count; ++i)
    {
        const GenericDrivePartition* part = &drive->partitions[i];

        KLOGF(
            INFO,
            "       %s%s%s [0x%08X-0x%08X] on %s",
            i == drive->partitions_count - 1 ? "\\_ " : "|- ",
            drive->name,
            "0",
            (u32)part->start,
            (u32)part->size + (u32)part->start,
            part->mountpoint ? part->mountpoint : "null");
    }
}

static void vfs_try_parse_boot_sector(GenericDrive* drive)
{
    if (mbr_drive_has_mbr(drive))
        mbr_parse_drive_info(drive);
}

bool vfs_add_drive(const GenericDrive* d)
{
    if (!d)
        return false;

    g_drives = krealloc(g_drives, (++g_drives_count) * sizeof(GenericDrive));
    if (!g_drives)
        panic("Falied to allocate VFS device!");

    GenericDrive* drive = &g_drives[g_drives_count - 1];
    /* We memcpy since there are some const members that we can't assign. */
    memcpy(drive, d, sizeof(GenericDrive));

    drive->uid = 0;
    drive->partitions_count = 0;
    drive->partitions = NULL;

    vfs_try_parse_boot_sector(drive);

    vfs_dump_drive_info(drive);

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

bool vfs_mount(const char* name, const char* mountpoint)
{
    return false;
}

bool vfs_umount(const char* name_or_mountpoint)
{
    return false;
}
