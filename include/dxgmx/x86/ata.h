/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_X86_ATA_H
#define _DXGMX_X86_ATA_H

#include<dxgmx/types.h>

#define ATA_DISK_SECTOR_SIZE 512

typedef struct S_ATADevice ATADevice;
typedef bool (*ata_device_read_func)(u64 start, u16 sectors, u8 *buf, const ATADevice *dev);
typedef bool (*ata_device_write_func)(u64 start, u16 sectors, const u8* buf, const ATADevice *dev);

typedef struct
S_ATADevice
{
    u16 bus_io;
    u16 bus_ctrl;
    u64 sector_count;
    ata_device_read_func read_func;
    ata_device_write_func write_func;
    
    union
    {
        u8 mode;
        struct
        {
            u8 master: 1;
            u8 chs: 1;
            u8 lba28: 1;
            u8 lba48: 1;
            u8 bigcable: 1;
            u8 dma: 3;
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
const ATADevice *ata_get_devices(size_t *count);

/**
 * @brief Reads from an ATA device, using the best available method (PIO or DMA).
 * 
 * @param start LBA from where to start reading on the disk.
 * @param sectors How many sectors to read (Note that a value of 0 means 256).
 * @param buf Destination buffer.
 * @param dev The ATA device.
 * @return true is successful, false otherwise. 
 */
bool ata_read(u64 start, u16 sectors, u8 *buf, const ATADevice *dev);
/**
 * @brief Writes to an ATA device using the best available method (PIO or DMA).
 * 
 * @param start LBA from where to start writing on the disk.
 * @param sectors How many sectors to write (Note that a value of 0 means 256).
 * @param buf Source data buffer.
 * @param dev The ATA device.
 * @return true is successful, false otherwise. 
 */
bool ata_write(u64 start, u16 sectors, const u8* buf, const ATADevice *dev);
/**
 * @brief Reads from an ATA device using PIO
 * 
 * @param start LBA from where to start reading on the disk.
 * @param sectors How many sectors to read (Note that a value of 0 means 256)..
 * @param buf Destination buffer.
 * @param dev The ATA device.
 * @return true is successful, false otherwise. 
 */
bool atapio_read(u64 start, u16 sectors, u8 *buf, const ATADevice *dev);
/**
 * @brief Writes to an ATA device using PIO.
 * 
 * @param start LBA from where to start writing on the disk.
 * @param sectors How many sectors to write (Note that a value of 0 means 256).
 * @param buf Source data buffer.
 * @param dev The ATA device.
 * @return true is successful, false otherwise. 
 */
bool atapio_write(u64 start, u16 sectors, const u8* buf, const ATADevice *dev);

#endif // !_DXGMX_X86_ATA_H
