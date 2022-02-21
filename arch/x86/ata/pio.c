/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/x86/ata.h>
#include<dxgmx/x86/portio.h>
#include<dxgmx/types.h>
#include<dxgmx/timer.h>
#include<dxgmx/klog.h>
#include<dxgmx/kmalloc.h>
#include<dxgmx/string.h>
#include<dxgmx/math.h>

#define KLOGF(lvl, fmt, ...) klogln(lvl, "ata: " fmt, ##__VA_ARGS__)

#define ATAPIO_READ 0x20
#define ATAPIO_WRITE 0x30
#define ATAPIO_READ_EXT 0x24
#define ATAPIO_WRITE_EXT 0x34
#define ATAPIO_FLUSH_SECTORS 0xE7

#define ATAPIO_READ_TIMEOUT_MS 200
#define ATAPIO_WRITE_TIMEOUT_MS 200
#define ATAPIO_FLUSH_SECTORS_TIMEOUT_MS 200

/**
 * @brief Converts 'sectors' to internal ATAPIO sectors.
 * ATAPIO drives take in an u8 as their sector count, which means
 * that the maximum number of sectors they can work with at once is capped at 256.
 * Note that ATAPIO drives interpret a sector count of 0 as 256.
 * @param sectors The normal sector count. Note that if a value greater than 256 is given,
 * it will be capped at 256.
 * @return The sector count that can be passed to the ATAPIO drive.
 */
static u8 atapio_internal_sectors(size_t sectors)
{
    return sectors >= 256 ? 0 : sectors;
}

/**
 * @brief Sends an ATAPIO read command to the given device.
 * 
 * @param lba From which LBA to start reading.
 * @param sectors How many sectors to read. (Need to be internal ATAPIO sectors: see atapip_internal_sectors()).
 * @param dev The device.
 * @return false: If the device is not a PIO device.
 */
static bool atapio_send_read_cmd(u64 lba, size_t sectors, const ATADevice *dev)
{
    if(dev->lba48)
    {
        port_outb(0x40 | (!dev->master << 4), ATA_DRIVE_SEL_REG(dev->bus_io));
        /* Send the higher halfs of the sector count and lba address. */
        port_outb(sectors >> 8, ATA_SECTOR_REG(dev->bus_io));
        port_outb(lba >> 24, ATA_LBA_LO_REG(dev->bus_io));
        port_outb(lba >> 32, ATA_LBA_MID_REG(dev->bus_io));
        port_outb(lba >> 40, ATA_LBA_HI_REG(dev->bus_io));
        /* Send the lower halfs of the sector count and lba address. */
        port_outb(sectors, ATA_SECTOR_REG(dev->bus_io));
        port_outb(lba, ATA_LBA_LO_REG(dev->bus_io));
        port_outb(lba >> 8, ATA_LBA_MID_REG(dev->bus_io));
        port_outb(lba >> 16, ATA_LBA_HI_REG(dev->bus_io));

        port_outb(ATAPIO_READ_EXT, ATA_COMMAND_REG(dev->bus_io));
    }
    else if(dev->lba28)
    {
        port_outb(0xE0 | (!dev->master << 4) | ((lba >> 24) & 0x0F), ATA_DRIVE_SEL_REG(dev->bus_io));

        port_outb(sectors, ATA_SECTOR_REG(dev->bus_io));

        port_outb(lba, ATA_LBA_LO_REG(dev->bus_io));
        port_outb(lba >> 8, ATA_LBA_MID_REG(dev->bus_io));
        port_outb(lba >> 16, ATA_LBA_HI_REG(dev->bus_io));

        port_outb(ATAPIO_READ, ATA_COMMAND_REG(dev->bus_io));
    }
    else
    {
        KLOGF(ERR, "atapio_read() called for non-PIO drive!");
        return false;
    }

    return true;
}

/**
 * @brief Sends an ATAPIO write command to the given device.
 * 
 * @param lba From which LBA to start writing.
 * @param sectors How many sectors to write. (Need to be internal ATAPIO sectors: see atapip_internal_sectors()).
 * @param dev The device.
 * @return false: If the device is not a PIO device.
 */
static bool atapio_send_write_cmd(u64 lba, size_t sectors, const ATADevice *dev)
{
    if(dev->lba48)
    {
        port_outb(0x40 | (!dev->master << 4), ATA_DRIVE_SEL_REG(dev->bus_io));
        /* Send the higher halfs of the sector count and lba address. */
        port_outb(sectors >> 8, ATA_SECTOR_REG(dev->bus_io));
        port_outb(lba >> 24, ATA_LBA_LO_REG(dev->bus_io));
        port_outb(lba >> 32, ATA_LBA_MID_REG(dev->bus_io));
        port_outb(lba >> 40, ATA_LBA_HI_REG(dev->bus_io));
        /* Send the lower halfs of the sector count and lba address. */
        port_outb(sectors, ATA_SECTOR_REG(dev->bus_io));
        port_outb(lba, ATA_LBA_LO_REG(dev->bus_io));
        port_outb(lba >> 8, ATA_LBA_MID_REG(dev->bus_io));
        port_outb(lba >> 16, ATA_LBA_HI_REG(dev->bus_io));

        port_outb(ATAPIO_WRITE_EXT, ATA_COMMAND_REG(dev->bus_io));
    }
    else if(dev->lba28)
    {
        port_outb(
            0xE0 | (!dev->master << 4) | ((lba >> 24) & 0x0F), 
            ATA_DRIVE_SEL_REG(dev->bus_io)
        );

        port_outb(sectors, ATA_SECTOR_REG(dev->bus_io));

        port_outb(lba, ATA_LBA_LO_REG(dev->bus_io));
        port_outb(lba >> 8, ATA_LBA_MID_REG(dev->bus_io));
        port_outb(lba >> 16, ATA_LBA_HI_REG(dev->bus_io));

        port_outb(ATAPIO_WRITE, ATA_COMMAND_REG(dev->bus_io));
    }
    else
    {
        KLOGF(ERR, "atapio_write() called for non-PIO drive!");
        return false;
    }

    return true;
}

static bool atapio_flush_written_sectors(time_t timeout_ms, const ATADevice *dev)
{
    port_outb(ATAPIO_FLUSH_SECTORS, ATA_COMMAND_REG(dev->bus_io));
    /* Wait for the sectors to actually flush. */
    Timer t;
    timer_start(&t);
    while(port_inb(ATA_STATUS_REG(dev->bus_io)) & ATAPIO_STATUS_BSY)
    {
        if(timer_ellapsed_ms(&t) > timeout_ms)
        {
            KLOGF(ERR, "Timed out trying to flush sectors to disk!");
            return false;
        }
    }

    return true;
}

static bool atapio_wait_for_ready(time_t timeout_ms, const ATADevice *dev)
{
    Timer t;
    timer_start(&t);
    while(true)
    {
        if(timer_ellapsed_ms(&t) > timeout_ms)
        {
            KLOGF(ERR, "Timed out trying to write to disk!");
            return false;
        }

        u8 status = port_inb(ATA_STATUS_REG(dev->bus_io));
        if( !(status & ATAPIO_STATUS_BSY) && (status & ATAPIO_STATUS_DRQ) )
            break;

        if( (status & ATAPIO_STATUS_ERR) || (status & ATAPIO_STATUS_DF) )
        {
            KLOGF(ERR, "Error trying to write to disk.");
            return false;
        }
    }

    return true;
}

static bool atapio_is_valid_range(u64 lba, size_t sectors, const ATADevice *dev)
{
    /* Check for possible underflow. */
    if(lba > dev->sector_count)
        return false;
    
    return (sectors <= dev->sector_count - lba);
}

bool atapio_read_sectors(u64 lba, size_t sectors, u8 *buf, const ATADevice *dev)
{
    if(!(dev && buf && sectors))
        return false;

    if(!atapio_is_valid_range(lba, sectors, dev))
    {
        KLOGF(ERR, "Out of range read!");
        return false;
    }

    while(sectors)
    {
        const size_t workingsectors = min(sectors, 256);

        if(!atapio_send_read_cmd(lba, atapio_internal_sectors(workingsectors), dev))
        {
            KLOGF(ERR, "Failed to read sector!");
            return false;
        }

        for(size_t sector = 0; sector < workingsectors; ++sector)
        {
            if(sector > 0)
            {
                const struct timespec ts = { .tv_nsec = 400, .tv_sec = 0 };
                nanosleep(&ts, NULL);
            }

            if(!atapio_wait_for_ready(ATAPIO_READ_TIMEOUT_MS, dev))
            {
                KLOGF(ERR, "Failed to read sector!");
                return false;
            }

            for(size_t word = 0; word < 256; ++word)
            {
                *((u16 *)buf) = port_inw(ATA_DATA_REG(dev->bus_io));
                buf += 2;
            }
        }

        sectors -= workingsectors;
        lba += workingsectors;
    }
    
    return true;
}

bool atapio_write_sectors(u64 lba, size_t sectors, const u8 *buf, const ATADevice *dev)
{
    if(!(dev && buf && sectors))
        return false;
    
    while(sectors)
    {
        const size_t workingsectors = min(sectors, 256);

        if(!atapio_send_write_cmd(lba, atapio_internal_sectors(workingsectors), dev))
        {
            KLOGF(ERR, "Failed to write sector!");
            return false;
        }

        const struct timespec ts = { .tv_nsec = 400, .tv_sec = 0 };
        for(size_t i = 0; i < workingsectors; ++i)
        {
            if(i)
                nanosleep(&ts, NULL);

            if(!atapio_wait_for_ready(ATAPIO_READ_TIMEOUT_MS, dev))
            {
                KLOGF(ERR, "Failed to write sector!");
                return false;
            }

            for(u16 k = 0; k < 256; ++k)
            {
                port_outw(*((u16 *)buf), ATA_DATA_REG(dev->bus_io));
                buf += 2;

                /* Sleep just for good measure. ? */
                nanosleep(&ts, NULL);
            }
        }

        if(!atapio_flush_written_sectors(ATAPIO_FLUSH_SECTORS_TIMEOUT_MS, dev))
        {
            KLOGF(ERR, "Timed-out trying to flush sectors to disk!");
            return false;
        }
        sectors -= workingsectors;
        lba += workingsectors;
    }

    return true;
}

bool atapio_read(u64 start, size_t n, u8 *buf, const ATADevice *dev)
{
    if(!(dev && buf && n))
        return false;

    u64 lba = start / ATA_DISK_SECTOR_SIZE;
    const size_t lbaoffset = start % ATA_DISK_SECTOR_SIZE;
    size_t leading_bytes = 0;
    if(lbaoffset)
    {
        leading_bytes = min(ATA_DISK_SECTOR_SIZE - lbaoffset, n);
        n -= leading_bytes;
    }
    size_t wholesectors = n / ATA_DISK_SECTOR_SIZE;
    size_t trailing_bytes = n % ATA_DISK_SECTOR_SIZE;

    if(!atapio_is_valid_range(lba, wholesectors + (bool)trailing_bytes + (bool)leading_bytes, dev))
    {
        KLOGF(ERR, "Out of range read!");
        return false;
    }

    if(leading_bytes)
    {
        if(
            !atapio_send_read_cmd(lba, 1, dev) ||
            !atapio_wait_for_ready(ATAPIO_READ_TIMEOUT_MS, dev)
        )
        {
            KLOGF(ERR, "Failed to read leading bytes!");
            return false;
        }

        for(u16 word = 0; word < 256; ++word)
        {
            u16 data = port_inw(ATA_DATA_REG(dev->bus_io));
            if(!leading_bytes)
                continue;

            /* There has to be a better way of doing this but i can t be fucked right now. */
            if(word * 2 >= lbaoffset)
            {
                if(leading_bytes == 1)
                {
                    *buf = data;
                    ++buf;
                    --leading_bytes;
                }
                else
                {
                    *((u16 *)buf) = data;
                    buf += 2;
                    leading_bytes -= 2;
                }
            }
            else if(word * 2 + 1 == lbaoffset)
            {
                *buf = data >> 8;
                ++buf;
                --leading_bytes;
            }
        }
        ++lba;
    }

    /* Read any whole sectors */
    if(wholesectors)
    {
        if(!atapio_read_sectors(lba, wholesectors, buf, dev))
            return false;
        lba += wholesectors;
        buf += wholesectors * ATA_DISK_SECTOR_SIZE;
    }

    /* Read any extra trailing bytes. */
    if(trailing_bytes)
    {
        // always read one sector
        if(
            !atapio_send_read_cmd(lba, 1, dev) ||
            !atapio_wait_for_ready(ATAPIO_READ_TIMEOUT_MS, dev)
        )
        {
            KLOGF(ERR, "Failed to read trailing bytes!");
            return false;
        }

        for(size_t i = 0; i < 256; ++i)
        {
            u16 data = port_inw(ATA_DATA_REG(dev->bus_io));
            if(!trailing_bytes)
                continue;

            if(trailing_bytes == 1)
            {
                /* last byte */
                *buf = data;
                --trailing_bytes;
            }
            else
            {
                *((u16 *)buf) = data;
                buf += 2;
                trailing_bytes -= 2;
            }
        }
    }

    return true;
}

bool atapio_write(u64 start, size_t n, const u8* buf, const ATADevice *dev)
{
    if(!(dev && buf && n))
        return false;

    u64 lba = start / ATA_DISK_SECTOR_SIZE;
    size_t lbaoffset = start % ATA_DISK_SECTOR_SIZE;
    size_t leading_bytes = 0;
    if(lbaoffset)
    {
        leading_bytes = min(ATA_DISK_SECTOR_SIZE - lbaoffset, n);
        n -= leading_bytes;
    }
    size_t wholesectors = n / ATA_DISK_SECTOR_SIZE;
    size_t trailing_bytes = n % ATA_DISK_SECTOR_SIZE;

    if(!atapio_is_valid_range(lba, wholesectors + (bool)trailing_bytes, dev))
    {
        KLOGF(ERR, "Out of range write!");
        return false;
    }

    const struct timespec ts = { .tv_nsec = 400, .tv_sec = 0 };

    /* If 'start' is not aligned on sector boundry. */
    if(leading_bytes)
    {
        u8 *oldsector = kmalloc(ATA_DISK_SECTOR_SIZE);
        if(!oldsector || !ata_read(lba * ATA_DISK_SECTOR_SIZE, ATA_DISK_SECTOR_SIZE, oldsector, dev))
        {
            KLOGF(ERR, "Failed to write leading bytes!");
            return false;
        }

        memcpy(oldsector + lbaoffset, buf, leading_bytes);
        buf += leading_bytes;

        if(
            !atapio_send_write_cmd(lba, 1, dev) ||
            !atapio_wait_for_ready(ATAPIO_WRITE_TIMEOUT_MS, dev)
        )
        {
            KLOGF(ERR, "Failed to write leading bytes!");
            kfree(oldsector);
            return false;
        }

        for(size_t i = 0; i < 256; ++i)
        {
            port_outw(((u16 *)oldsector)[i], ATA_DATA_REG(dev->bus_io));
            nanosleep(&ts, NULL);
        }

        kfree(oldsector);

        if(!atapio_flush_written_sectors(ATAPIO_FLUSH_SECTORS_TIMEOUT_MS, dev))
        {
            KLOGF(ERR, "Timed-out trying to flush sectors to disk!");
            return false;
        }

        ++lba;
    }

    if(wholesectors)
    {
        if(!atapio_write_sectors(lba, wholesectors, buf, dev))
        {
            return false;
        }
            
        lba += wholesectors;
        buf += wholesectors * ATA_DISK_SECTOR_SIZE;
    }

    /* Write any extra bytes. */
    if(trailing_bytes)
    {
        /* If we got here that means we have to write a fraction of a sector.
        Which in turn means we have to read the current corresponding sector, 
        modify it with the new data, and write it back to disk. */
        u8 *oldsector = kmalloc(ATA_DISK_SECTOR_SIZE);
        if(
            !oldsector || 
            !atapio_read(lba * ATA_DISK_SECTOR_SIZE, ATA_DISK_SECTOR_SIZE, oldsector, dev)
        )
        {
            KLOGF(ERR, "Failed to write trailing bytes!");
            kfree(oldsector);
            return false;
        }

        memcpy(oldsector, buf, trailing_bytes);

        if(
            !atapio_send_write_cmd(lba, 1, dev) ||
            !atapio_wait_for_ready(ATAPIO_READ_TIMEOUT_MS, dev)
        )
        {
            KLOGF(ERR, "Failed to write trailing bytes!");
            kfree(oldsector);
            return false;
        }

        for(size_t i = 0; i < 256; ++i)
        {
            port_outw(((u16 *)oldsector)[i], ATA_DATA_REG(dev->bus_io));
            /* Sleep just for good measure. ? */
            nanosleep(&ts, NULL);
        }

        kfree(oldsector);

        if(!atapio_flush_written_sectors(ATAPIO_FLUSH_SECTORS_TIMEOUT_MS, dev))
        {
            KLOGF(ERR, "Timed-out trying to flush sectors to disk!");
            return false;
        }
    }

    return true;
}
