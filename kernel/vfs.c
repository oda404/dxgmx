/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/panic.h>
#include <dxgmx/stdio.h>
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
        drive->physical_sectorsize);

    for (size_t i = 0; i < drive->partitions_count; ++i)
    {
        const GenericDrivePartition* part = &drive->partitions[i];
        const u32 start =
            part->lba_start * part->parent_drive->physical_sectorsize;
        const u32 end =
            part->sectors_count * part->parent_drive->physical_sectorsize +
            start;

        KLOGF(
            INFO,
            "       %s%s%s [0x%08X-0x%08X] on %s",
            i == drive->partitions_count - 1 ? "\\_ " : "|- ",
            drive->name,
            part->suffix,
            start,
            end,
            part->mountpoint ? part->mountpoint : "null");
    }
}

static void vfs_try_parse_boot_sector(GenericDrive* drive)
{
    if (mbr_drive_has_mbr(drive))
        mbr_parse_drive_info(drive);
}

static bool vfs_generate_partition_suffix(GenericDrivePartition* part)
{
    /* For now all partition follow the same naming scheme: p%u */
    part->suffix = kmalloc(3);
    if (!part->suffix)
        return false;

    snprintf(part->suffix, 3, "p%.1u", part->number);

    return true;
}

static bool vfs_generic_part_read(
    u64 relstart, size_t n, void* buf, const GenericDrivePartition* part)
{
    const GenericDrive* drive = part->parent_drive;
    return drive->read(
        part->lba_start * drive->physical_sectorsize + relstart,
        n,
        buf,
        drive->internal_dev);
}

static bool vfs_generic_part_write(
    u64 relstart, size_t n, const void* buf, const GenericDrivePartition* part)
{
    const GenericDrive* drive = part->parent_drive;
    return drive->write(
        part->lba_start * drive->physical_sectorsize + relstart,
        n,
        buf,
        drive->internal_dev);
}

bool vfs_add_drive(const GenericDrive* d)
{
    if (!d)
        return false;

    g_drives = krealloc(g_drives, (++g_drives_count) * sizeof(GenericDrive));
    if (!g_drives)
        panic("Failed to allocate VFS device!");

    GenericDrive* drive = &g_drives[g_drives_count - 1];
    /* We memcpy since there are some const members that we can't assign. */
    memcpy(drive, d, sizeof(GenericDrive));

    drive->logical_sectorsize = drive->physical_sectorsize;
    drive->uid = 0;
    drive->partitions_count = 0;
    drive->partitions = NULL;

    /** From the boot sector we should get the following information (if valid):
     * drive->uid,
     * drive->partition_count,
     * drive->partitions
     *
     * And in turn each partition should have the following information set:
     * partition->start,
     * partition->size,
     * partition->number
     *
     * The rest is up to us to fill in.
     */
    vfs_try_parse_boot_sector(drive);

    for (size_t i = 0; i < drive->partitions_count; ++i)
    {
        GenericDrivePartition* part = &drive->partitions[i];
        part->parent_drive = drive;
        part->read = vfs_generic_part_read;
        part->write = vfs_generic_part_write;
        vfs_generate_partition_suffix(part);
    }

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
