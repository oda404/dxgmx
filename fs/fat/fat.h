/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_FS_FAT_FAT_H
#define _DXGMX_FS_FAT_FAT_H

/* FAT32 filesystem driver. */

#include <dxgmx/assert.h>
#include <dxgmx/compiler_attrs.h>
#include <dxgmx/fs/fs.h>
#include <dxgmx/types.h>
#include <posix/sys/stat.h>

/* Fat files don't support permissions so they are all RWX R-X R-X, user, group,
 * others */
#define FAT_FILE_MODE (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)
#define FAT_DIR_MODE (FAT_FILE_MODE | S_IFDIR)

#define FAT32_CTX(fs)                                                          \
    ((FAT32Ctx*)fs->driver_ctx);                                               \
    ASSERT(fs->driver_ctx)

/* FATEntry attributes. */
/* Read-only, writing must fail. */
#define FAT_ENTRY_RO 0x01
/* Entry is hidden unless otherwise specifed. */
#define FAT_ENTRY_HIDDEN 0x02
/* File is part of OS, and must not be listed unless otherwise specified ?? */
#define FAT_ENTRY_SYSTEM 0x04
/* The volume label, can only be present on the root dir. */
#define FAT_ENTRY_VOL_ID 0x08
/* Entry is a directory. */
#define FAT_ENTRY_DIR 0x10
/* Entry is an archive, I still don't completely understand what this is... */
#define FAT_ENTRY_ARCHIVE 0x20
/* Entry is a Long File Name entry. */
#define FAT_ENTRY_LFN 0x0F

typedef enum E_FatType
{
    FAT12,
    FAT16,
    FAT32
} FatType;

/* The FAT BiosParameterBlock */
typedef struct _ATTR_PACKED S_FatBootRecord
{
    /* Opcodes to jump to the bootcode. */
    u8 jump_boot[3];
    /* System that formatted the volume ? */
    u8 oem_name[8];

    /* Is this always the sector size of the underlying disk ? */
    u16 sectorsize;

    /* Number of sectors per cluster. */
    u8 sectors_per_cluster;

    /* How many sectors are reserved, meaning the BPB region ? */
    u16 reserved_sector_count;

    /* The number of File Allocation Tables, usually 2 */
    u8 fat_count;

    /* 32-byte entry count in the root dir. Always 0 for FAT32. */
    u16 root_dir_entries_count;

    /* Number of sectors on the entire volume. Always 0 for FAT32 */
    u16 sector_count16;

    /* Media type.*/
    u8 media;

    /* How many sectors one FAT occupies. Always 0 for FAT32 */
    u16 sectors_per_fat16;

    /* Geometry for INT 0x13 */
    u16 sectors_per_track;
    /* Geometry for INT 0x13 */
    u16 heads_count;
    /* Geometry for INT 0x13 */
    u32 hidden_sectors_count;
    /* Total sectors on volume. Must be > 0 set for FAT32.. */
    u32 sectors_count32;
} FatBootRecord;

typedef struct _ATTR_PACKED S_Fat32BootRecord
{
    /* How many sectors one FAT occupies. Must be > 0 for FAT32 */
    u32 sectors_per_fat32;

    /* FIXME: flags */
    u16 flags;

    /* Must be 0 */
    u16 version;

    /* Cluster number of the first cluster of the root dir. */
    u32 root_dir_cluster;

    /* Sector number of the FAT32FSInfo structure in the reserved area of the
     * volume. */
    u16 fsinfo_sector;

    /* Sector number of the backup bootsector in the reserved area of the
     * volume. Allegedly can be only 0 or 6. Valid if > 0. */
    u16 backup_bootsector_sector;

    /* Must be 0 */
    u8 reserved[12];

    /* Driver number for INT 0x13. 0x0 or 0x80 */
    u8 drive_number;

    /* 0x0 */
    u8 ntflags;

    u8 signature;
    u32 volume_id;
    u8 volume_label[11];
    u8 system_identifier[8];

    // ... boot code and boot partition signature.
} Fat32BootRecord;

typedef struct _ATTR_PACKED S_Fat32FSInfo
{
    u32 lead_signature;
    u8 reserved[480];
    u32 second_signature;
    u32 last_free_cluster;
    u32 start_cluster;
    u8 reserved2[12];
    u32 trail_signature;
} Fat32FSInfo;

typedef struct _ATTR_PACKED S_FATEntryTime
{
    u8 hour : 5;
    u8 min : 6;
    u8 sec_div2 : 5;
} FATEntryTime;

typedef struct _ATTR_PACKED S_FATEntryDate
{
    u8 year : 7;
    u8 month : 4;
    u8 day : 5;
} FATEntryDate;

typedef struct _ATTR_PACKED S_FATEntry
{
    char name[8];
    char ext[3];
    u8 attributes;
    u8 ntreserved;
    u8 creation_tenths_of_sec;
    FATEntryTime creation_time;
    FATEntryDate creation_date;
    FATEntryDate last_access_date;
    u16 cluster_hi;
    FATEntryDate last_write_time;
    FATEntryDate last_write_date;
    u16 cluster_lo;
    u32 file_size;
} FATEntry;

typedef struct _ATTR_PACKED S_FATLFNEntry
{
    u8 order;
    u16 name1[5];
    u8 attributes;
    u8 type;
    u8 checksum;
    u16 name2[6];
    u16 reserved;
    u16 name3[2];
} FATLFNEntry;

/* The FAT32 driver context. */
typedef struct S_FAT32Ctx
{
    /* Size of a sector. If this is somehow different than the blkdev's
     * sectorsize we reject the fs. */
    u16 sectorsize;
    /* How many sectors one FAT occupies. */
    u32 sectors_per_fat;
    /* How many sectors are in a cluster */
    u8 sectors_per_cluster;
    /* How many sectors there are on the volume. */
    u32 sector_count;
    /* How many sectors there are in the data region. */
    u32 data_sector_count;
    /* How many clusters there are on the volume. A cluster is a grouping
    of 'sectors_per_cluster' sectors that hold the data of a file/directory. */
    u32 cluster_count;
    /* How many sectors the root directories occupy */
    u16 root_dir_sector_count;
    /* The first cluster of the root dir. */
    u32 root_dir_cluster;
    /* Includes BPB */
    u16 reserved_sector_count;
    /* The first disk sector that can be used for files. */
    u32 first_data_sector;
    /* Fat type 12/16/32 */
    FatType type;
    /* The size of a cluster. */
    size_t clustersize;
    /* Maximum number of entries in a single directory cluster. */
    size_t entries_per_clusterdir;
    /* The blkdevice backing this filesystem. */
    const BlockDevice* blkdev;
} FAT32Ctx;

/* Struct that represents a position when reading an entry. */
typedef struct S_FATEntryLoc
{
    /* The cluster we are currently reading */
    u32 cluster;
    /* Entry offset in the above cluster */
    u8 offset;
    /* True if offset rolled over, and a new (next) cluster should be read. */
    bool read_next;
} FATEntryLoc;

/* FAT32 read function. */
ssize_t fat_read(const VirtualNode* vnode, void* buf, size_t n, off_t off);

/**
 * Cache the vnode for the root dir for a FAT32 filesystem
 *
 * 'fs' Target filesystem.
 *
 * Returns:
 * 0 on success.
 * -ENOMEM on out of memory.
 */
int fat_cache_root_vnode(FileSystem* fs);

/**
 * Enumerate a directory and cache all of it's entries. This function recurses
 * for all sub-directories.
 *
 * 'dir_vnode' The vnode of the dir.
 * 'fs' The filesystem.
 *
 * Returns:
 * 0 on success.
 * Negative values representing an errno.
 */
int fat_enumerate_and_cache_dir(const VirtualNode* dir_vnode, FileSystem* fs);

/* Cluster is not used, available */
#define FAT_CLUSTER_FREE 0
/* Cluster is in use, but is not EOF */
#define FAT_CLUSTER_USED 1
/* Cluster is reserved and should not be used. */
#define FAT_CLUSTER_RESERVED 2
/* Cluster is defective */
#define FAT_CLUSTER_BAD 3
/* Cluster is in use, and is EOF */
#define FAT_CLUSTER_EOF 4
/* Cluster number is not valid. */
#define FAT_CLUSTER_BROKEN 5

/**
 * Check with the FAT to see what kind of cluster we have.
 *
 * 'cluster' The cluster.
 * 'buf' Buffer big enough to read one disk sector (for FAT reading).
 * 'ctx' The ctx.
 *
 * Returns:
 * A positive integer on success (see FAT_CLUSTER_*).
 * A negative errno if there wes a problem reading from the disk.
 */
int fat_cluster_type(u32 cluster, u8* buf, const FAT32Ctx* ctx);

/**
 * Read a cluster. Doesn't do any error checking. It assumes you verified
 * the cluster you are trying to read from is valid.
 *
 * 'buf' Destination buffer. Needs to be large enough to store a cluster for
 * this ctx.
 * 'loc' The location of the cluster.
 * 'ctx' The ctx.
 *
 * Returns:
 * 0 on sucess.
 * Negative errnos coming from the disk on error.
 */
int fat_read_one_cluster(u8* buf, u32 clus, const FAT32Ctx* ctx);

/**
 * Advance the entry loc by one. If the entry loc reaches the maximum
 * value it is set to 0 and signals that the next cluster needs to
 * be read by setting loc->read_next.
 *
 * 'ctx' Thex ctx.
 * 'loc' The target location.
 */
void fat_advance_loc(const FAT32Ctx* ctx, FATEntryLoc* loc);

/**
 * Advance the entry loc by one cluster. This function reads the fat entry for
 * the current cluster, trying to determine the value of the next one. If
 * successul loc->read_next will be cleared.
 *
 * 'loc' The target location.
 * 'buf' Buffer big enough to store a disk sector (for FAT reading).
 * 'ctx' The ctx.
 *
 * Returns:
 * 0 on sucess.
 * -EINVAL if there is no *valid* cluster following the current one.
 */
int fat_advance_loc_clus(FATEntryLoc* loc, u8* buf, const FAT32Ctx* ctx);

/**
 * Copy a specified FATEntry from 'clustersrc' into 'entry'.
 *
 * 'clustersrc' A buffer holding a cluster.
 * 'off' The idx of the FATEntry we want to read from 'clustersrc'.
 * 'entry' Destination entry.
 */
void fat_read_one_entry(u8* clustersrc, size_t idx, FATEntry* entry);

/**
 * Free a vnode that has been set by fat_vnode_from_loc.
 *
 * 'vnode' The vnode to free.
 *
 * Returns:
 * 0 on success.
 */
int fat_free_vnode(VirtualNode* vnode);

/**
 * Find the cluster coming after the current one.
 *
 * 'clus' The current cluster. The new one will be writen here.
 * 'buf' A buffer big enough to read a disk sector into.
 * 'ctx' The ctx.
 *
 * Returns:
 * 0 if 'clus' has been set to the next cluster value.
 * -EINVAL if there is no *valid* cluster following this one, and 'clus' has not
 * been modified.
 */
int fat_next_clus(u32* clus, u8* buf, const FAT32Ctx* ctx);

/**
 * Transform a cluster number to a physical sector on disk.
 *
 * 'cluster' The cluster to translate.
 * 'ctx' The ctx.
 *
 * Returns:
 * The sector.
 */
u32 fat_cluster_to_sector(u32 cluster, const FAT32Ctx* ctx);

#endif // !_DXGMX_FS_FAT_FAT_H
