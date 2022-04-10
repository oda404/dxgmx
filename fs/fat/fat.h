/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_FS_FAT_FAT_H
#define _DXGMX_FS_FAT_FAT_H

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/types.h>

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
    u16 reserved_sectors_count;
    /* The number of File Allocation Tables, usually 2 */
    u8 fats_count;
    /* How many 32-byte entries there are in the root dir. Always 0 for Fat32.
     */
    u16 root_dir_entries_count;
    /* Number of sectors on the entire volume. Always 0 for FAT32 */
    u16 sectors_count16;
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
    /* Total sectors on volume. Must be set for FAT32, cleared othetwise. */
    u32 sectors_count32;
} FatBootRecord;

typedef struct _ATTR_PACKED S_Fat12BootRecord
{
    u8 drive_number;
    u8 ntflags;
    u8 signature;
    u32 volume_id;
    u8 volume_label[11];
    u8 system_identifier[8];

    // ... boot code and boot partition signature.
} Fat12BootRecord;

typedef Fat12BootRecord Fat16BootRecord;

typedef struct _ATTR_PACKED S_Fat32BootRecord
{
    u32 sectors_per_fat32;
    u16 flags;
    u8 version_min;
    u8 version_maj;
    u32 root_dir_cluster;
    u16 fsinfo_sector;
    u16 backup_bootsector_sector;
    u8 reserved[12];
    u8 drive_number;
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

#endif // !_DXGMX_FS_FAT_FAT_H
