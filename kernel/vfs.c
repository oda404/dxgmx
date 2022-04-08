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

#define KLOGF(lvl, fmt, ...) klogln(lvl, "vfs: " fmt __VA_OPT__(, ) __VA_ARGS__)

static GenericDrive* g_drives = NULL;
static size_t g_drives_count = 0;

static void vfs_dump_drive_info(const GenericDrive* drive)
{
    KLOGF(
        INFO,
        "%s: (UID 0x%X) sector size: %u, type: %s",
        drive->dev->name,
        (u32)drive->uid,
        drive->dev->physical_sectorsize,
        drive->dev->type);

    for (size_t i = 0; i < drive->partitions_count; ++i)
    {
        const GenericDrivePartition* part = &drive->partitions[i];
        const u32 start =
            part->lba_start * part->parent_drive->dev->physical_sectorsize;
        const u32 end =
            part->sectors_count * part->parent_drive->dev->physical_sectorsize +
            start;

        KLOGF(
            INFO,
            "   %s%s%s [0x%08X-0x%08X] on %s",
            i == drive->partitions_count - 1 ? "\\_ " : "|- ",
            drive->dev->name,
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
    lba_t relative_lba,
    sector_t sectors,
    void* buf,
    const GenericDrivePartition* part)
{
    const GenericDrive* drive = part->parent_drive;
    return drive->dev->read(
        part->lba_start + relative_lba, sectors, buf, drive->dev);
}

static bool vfs_generic_part_write(
    lba_t relative_lba,
    sector_t sectors,
    const void* buf,
    const GenericDrivePartition* part)
{
    const GenericDrive* drive = part->parent_drive;
    return drive->dev->write(
        part->lba_start + relative_lba, sectors, buf, drive->dev);
}

static GenericDrive* vfs_new_drive()
{
    GenericDrive* tmp =
        krealloc(g_drives, (g_drives_count + 1) * sizeof(GenericDrive));
    if (!tmp)
        return NULL;

    ++g_drives_count;
    g_drives = tmp;
    return &g_drives[g_drives_count - 1];
}

bool vfs_add_drive(const GenericStorageDevice* dev)
{
    if (!dev)
        return false;

    GenericDrive* drive = vfs_new_drive();
    if (!drive)
        return false;
    /* We memcpy since there are some const members that we can't assign. */
    drive->dev = dev;
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
