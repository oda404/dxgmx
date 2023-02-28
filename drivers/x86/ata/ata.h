/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_X86_ATA_H
#define _DXGMX_X86_ATA_H

#include <dxgmx/pci.h>
#include <dxgmx/storage/blkdev.h>
#include <dxgmx/types.h>

#define ATA_DEVICE_CTRL_PORT 0x3F6
#define ATA_ALTERNATE_STATUS_PORT 0x376

/* Error */
#define ATA_STATUS_ERR (1 << 0)
/* Index, 0 */
#define ATA_STATUS_IDX (1 << 1)
/* Corrected data, 0 */
#define ATA_STATUS_CORR (1 << 2)
/* Set when drive is ready to accept new data, or has data to transfer */
#define ATA_STATUS_DRQ (1 << 3)
/* Overlapped mode service request */
#define ATA_STATUS_SRV (1 << 4)
/* Drive fault. */
#define ATA_STATUS_DF (1 << 5)
/* Ready */
#define ATA_STATUS_RDY (1 << 6)
/* Busy */
#define ATA_STATUS_BSY (1 << 7)

/* Address Mark Not Found */
#define ATA_ERR_AMNF (1 << 0)
/* TracK Zero Not Found */
#define ATA_ERR_TKZNF (1 << 1)
/* Command aborted */
#define ATA_ERR_ABRT (1 << 2)
/* Media Change Request */
#define ATA_ERR_MCR (1 << 3)
/* ID Not Found */
#define ATA_ERR_IDNF (1 << 4)
/* Media changed */
#define ATA_ERR_MC (1 << 5)
/* Uncorrectable data error */
#define ATA_ERR_UNC (1 << 6)
/* Bad BlocK */
#define ATA_ERR_BBK (1 << 7)

#define ATA_REG_DATA(x) (x)
#define ATA_REG_ERR(x) (x + 1)
#define ATA_REG_FEATURES(x) (x + 1)
#define ATA_REG_SECTOR(x) (x + 2)
#define ATA_REG_LBA_LO(x) (x + 3)
#define ATA_REG_LBA_MID(x) (x + 4)
#define ATA_REG_LBA_HI(x) (x + 5)
#define ATA_REG_DRIVE_SEL(x) (x + 6)
#define ATA_REG_STATUS(x) (x + 7)
#define ATA_REG_COMMAND(x) (x + 7)

/* Called with the device control register port. */
#define ATA_REG_ALT_STATUS(x) (x)
#define ATA_REG_DEV_CTRL(x) (x)
#define ATA_REG_DRIVE_ADDR(x) (x + 1)

#define ATA_TYPE_UNKNOWN 0
#define ATA_TYPE_CHS 1
#define ATA_TYPE_LBA28 2
#define ATA_TYPE_LBA48 3

#define ATA_CMD_READ_PIO 0x20
#define ATA_CMD_READ_PIO_EXT 0x24
#define ATA_CMD_READ_DMA 0xC8
#define ATA_CMD_READ_DMA_EXT 0x25
#define ATA_CMD_WRITE_PIO 0x30
#define ATA_CMD_WRITE_PIO_EXT 0x34
#define ATA_CMD_WRITE_DMA 0xCA
#define ATA_CMD_WRITE_DMA_EXT 0x35
#define ATA_CMD_CACHE_FLUSH 0xE7
#define ATA_CMD_CACHE_FLUSH_EXT 0xEA
#define ATA_CMD_PACKET 0xA0
#define ATA_CMD_IDENT_PACKET 0xA1
#define ATA_CMD_IDENT 0xEC

typedef struct S_AtaStorageDevice
{
    /* The ATA I/O port base */
    u16 portio;
    /* The ATA control port base */
    u16 portctl;
    /* Set if this is a master device. */
    u8 master : 1;
    /* See ATA_TYPE */
    u8 type : 2;
    /* Set if drive supports dma */
    u8 dma : 1;

    PCIDevice* controller;
} AtaStorageDevice;

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

const char* ata_error_to_str(u8 error);

#endif // !_DXGMX_X86_ATA_H
