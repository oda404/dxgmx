/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_X86_ATA_H
#define _DXGMX_X86_ATA_H

#include <dxgmx/storage/blkdev.h>
#include <dxgmx/types.h>

#define ATA_DEVICE_CTRL_PORT 0x3F6
#define ATA_ALTERNATE_STATUS_PORT 0x376

#define ATAPIO_STATUS_ERR 1
#define ATAPIO_STATUS_DRQ (1 << 3)
#define ATAPIO_STATUS_SRV (1 << 4)
#define ATAPIO_STATUS_DF (1 << 5)
#define ATAPIO_STATUS_RDY (1 << 6)
#define ATAPIO_STATUS_BSY (1 << 7)

#define ATA_HEAD_PORT(x) (x + 6)
#define ATA_STATUS_PORT(x) (x + 7)

#define ATA_ALT_STATUS_PORT(x) (x + 0)

#define ATA_DATA_REG(x) (x)
#define ATA_ERR_REG(x) (x + 1)
#define ATA_FEATURES_REG(x) (x + 1)
#define ATA_SECTOR_REG(x) (x + 2)
#define ATA_LBA_LO_REG(x) (x + 3)
#define ATA_LBA_MID_REG(x) (x + 4)
#define ATA_LBA_HI_REG(x) (x + 5)
#define ATA_DRIVE_SEL_REG(x) (x + 6)
#define ATA_STATUS_REG(x) (x + 7)
#define ATA_COMMAND_REG(x) (x + 7)

#define ATA_ALT_STATUS_REG(x) (x)
#define ATA_DEV_CTRL_REG(x) (x)
#define ATA_DRIVE_ADDR_REG(x) (x + 1)

#define ATA_TYPE_UNKNOWN 0
#define ATA_TYPE_CHS 1
#define ATA_TYPE_LBA28 2
#define ATA_TYPE_LBA48 3

typedef u16 atabus_t;

typedef struct S_AtaStorageDevice
{
    atabus_t bus_io;
    atabus_t bus_ctrl;
    u8 master : 1;
    u8 type : 2;
    u8 bigcable : 1;
    u8 dma : 3;
} AtaStorageDevice;

int ata_init();

ssize_t
atapio_read(const BlockDevice* dev, lba_t lba, sectorcnt_t sectors, void* dest);
/**
 * @brief Writes sectors to an ATA device using PIO.
 *
 * @param lba LBA from which to start writing.
 * @param sectors How many sectors to write.
 * @param buf Source buffer.
 * @param dev The ATA device.
 * @return true if successful, false otherwise.
 */
ssize_t atapio_write(
    const BlockDevice* dev, lba_t lba, sectorcnt_t sectors, const void* src);

#endif // !_DXGMX_X86_ATA_H
