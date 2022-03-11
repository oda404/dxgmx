/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_X86_ATA_H
#define _DXGMX_X86_ATA_H

#include <dxgmx/storage/drive.h>
#include <dxgmx/types.h>

#define ATA_DISK_SECTOR_SIZE 512

#define ATA_DEVICE_CTRL_PORT 0x3F6
#define ATA_ALTERNATE_STATUS_PORT 0x376

#define ATAPIO_STATUS_ERR 1
#define ATAPIO_STATUS_DRQ (1 << 3)
#define ATAPIO_STATUS_SRV (1 << 4)
#define ATAPIO_STATUS_DF (1 << 5)
#define ATAPIO_STATUS_RDY (1 << 6)
#define ATAPIO_STATUS_BSY (1 << 7)

#define ATA_IDENTIFY 0xEC

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

typedef struct S_ATADevice
{
    char* name;
    size_t namelen;

    u16 bus_io;
    u16 bus_ctrl;
    u64 sector_count;
    storage_drive_read read;
    storage_drive_write write;

    union
    {
        u8 mode;
        struct
        {
            u8 master : 1;
            u8 chs : 1;
            u8 lba28 : 1;
            u8 lba48 : 1;
            u8 bigcable : 1;
            u8 dma : 3;
        };
    };
} ATADevice;

int ata_init();
/**
 * @brief Returns an array of all the ATA devices.
 *
 * @param count Where to put the number of ATA devices.
 * @return The array of ATA devices. NULL if there aren't any.
 */
const ATADevice* ata_get_devices(size_t* count);

/**
 * @brief Reads from an ATA device, using the best available method (PIO or
 * DMA).
 *
 * @param start Byte address from which to start reading.
 * @param n How many bytes to read.
 * @param buf Destination buffer.
 * @param dev The ATA device.
 * @return true if successful, false otherwise.
 */
bool ata_read(u64 start, size_t n, u8* buf, const ATADevice* dev);
/**
 * @brief Writes to an ATA device using the best available method (PIO or DMA).
 *
 * @param start Byte address from which to start writing.
 * @param n How many bytes to write.
 * @param buf Source data buffer.
 * @param dev The ATA device.
 * @return true if successful, false otherwise.
 */
bool ata_write(u64 start, size_t n, const u8* buf, const ATADevice* dev);
/**
 * @brief Reads from an ATA device using PIO
 *
 * @param start Byte address from which to start reading.
 * @param n How many bytes to read.
 * @param buf Destination buffer.
 * @param dev The ATA device.
 * @return true if successful, false otherwise.
 */
bool atapio_read(u64 start, size_t n, u8* buf, const ATADevice* dev);
/**
 * @brief Writes to an ATA device using PIO.
 *
 * @param start Byte address from which to start writing.
 * @param n How many bytes to write.
 * @param buf Source data buffer.
 * @param dev The ATA device.
 * @return true if successful, false otherwise.
 */
bool atapio_write(u64 start, size_t n, const u8* buf, const ATADevice* dev);
/**
 * @brief Reads sectors from an ATA device using PIO.
 *
 * @param lba LBA address from which to start reading.
 * @param sectors How many sectors to read.
 * @param buf Destination buffer.
 * @param dev The ATA device.
 * @return true if successful, false otherwise.
 */
bool atapio_read_sectors(
    u64 lba, size_t sectors, u8* buf, const ATADevice* dev);
/**
 * @brief Writes sectors to an ATA device using PIO.
 *
 * @param lba LBA address from which to start writing.
 * @param sectors How many sectors to write.
 * @param buf Source buffer.
 * @param dev The ATA device.
 * @return true if successful, false otherwise.
 */
bool atapio_write_sectors(
    u64 lba, size_t sectors, const u8* buf, const ATADevice* dev);

#endif // !_DXGMX_X86_ATA_H
