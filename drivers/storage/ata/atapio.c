/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include "ata.h"
#include <dxgmx/assert.h>
#include <dxgmx/errno.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/math.h>
#include <dxgmx/string.h>
#include <dxgmx/timer.h>
#include <dxgmx/types.h>
#include <dxgmx/x86/portio.h>

#define KLOGF_PREFIX "atapio: "

#define ATA_READ_TIMEOUT_MS 200
#define ATA_WRITE_TIMEOUT_MS 200
#define ATA_FLUSH_SECTORS_TIMEOUT_MS 200

/**
 * @brief Converts 'sectors' to internal ATAPIO sectors.
 * ATAPIO drives take in an u8 as their sector count, which means
 * that the maximum number of sectors they can work with at once is capped at
 * 256. Note that ATAPIO drives interpret a sector count of 0 as 256.
 * @param sectors The normal sector count. Note that if a value greater than 256
 * is given, it will be capped at 256.
 * @return The sector count that can be passed to the ATAPIO drive.
 */
static u16 atapio_internal_sectors(sectorcnt_t sectors)
{
    return sectors >= 256 ? 0 : sectors;
}

/**
 * @brief Sends an ATAPIO read command to the given device.
 *
 * @param lba LBA from which to start reading.
 * @param sectors How many sectors to read. (In internal ATAPIO sectors:
 * see atapio_internal_sectors()).
 * @param dev The device.
 * @return false: If the device is not a PIO device.
 */
static bool atapio_send_read_cmd(lba_t lba, u16 sectors, const BlockDevice* dev)
{
    const AtaStorageDevice* atadev = dev->extra;

    switch (atadev->type)
    {
    case ATA_TYPE_LBA48:
        port_outb(
            0x40 | (!atadev->master << 4), ATA_REG_DRIVE_SEL(atadev->portio));
        /* Send the higher halfs of the sector count and lba. */
        port_outb(sectors >> 8, ATA_REG_SECTOR(atadev->portio));
        port_outb(lba >> 24, ATA_REG_LBA_LO(atadev->portio));
        port_outb(lba >> 32, ATA_REG_LBA_MID(atadev->portio));
        port_outb(lba >> 40, ATA_REG_LBA_HI(atadev->portio));
        /* Send the lower halfs of the sector count and lba. */
        port_outb(sectors, ATA_REG_SECTOR(atadev->portio));
        port_outb(lba, ATA_REG_LBA_LO(atadev->portio));
        port_outb(lba >> 8, ATA_REG_LBA_MID(atadev->portio));
        port_outb(lba >> 16, ATA_REG_LBA_HI(atadev->portio));
        /* Read extended command */
        port_outb(0x24, ATA_REG_COMMAND(atadev->portio));
        break;

    case ATA_TYPE_LBA28:
        port_outb(
            0xE0 | (!atadev->master << 4) | ((lba >> 24) & 0x0F),
            ATA_REG_DRIVE_SEL(atadev->portio));

        port_outb(sectors, ATA_REG_SECTOR(atadev->portio));

        port_outb(lba, ATA_REG_LBA_LO(atadev->portio));
        port_outb(lba >> 8, ATA_REG_LBA_MID(atadev->portio));
        port_outb(lba >> 16, ATA_REG_LBA_HI(atadev->portio));
        /* Read command. */
        port_outb(0x20, ATA_REG_COMMAND(atadev->portio));
        break;

    case ATA_TYPE_CHS:
        ASSERT_NOT_HIT(); // no

    default:
        KLOGF(ERR, "%s() - [%s] invalid ATA device!", __FUNCTION__, dev->name);
        return -EINVAL;
    }

    return sectors;
}

/**
 * @brief Sends an ATAPIO write command to the given device.
 *
 * @param lba From which LBA to start writing.
 * @param sectors How many sectors to write. (In internal ATAPIO sectors:
 * see atapio_internal_sectors()).
 * @param dev The device.
 * @return false: If the device is not a PIO device.
 */
static bool
atapio_send_write_cmd(lba_t lba, u16 sectors, const BlockDevice* dev)
{
    const AtaStorageDevice* atadev = dev->extra;

    switch (atadev->type)
    {
    case ATA_TYPE_LBA48:
        port_outb(
            0x40 | (!atadev->master << 4), ATA_REG_DRIVE_SEL(atadev->portio));
        /* Send the higher halfs of the sector count and lba. */
        port_outb(sectors >> 8, ATA_REG_SECTOR(atadev->portio));
        port_outb(lba >> 24, ATA_REG_LBA_LO(atadev->portio));
        port_outb(lba >> 32, ATA_REG_LBA_MID(atadev->portio));
        port_outb(lba >> 40, ATA_REG_LBA_HI(atadev->portio));
        /* Send the lower halfs of the sector count and lba. */
        port_outb(sectors, ATA_REG_SECTOR(atadev->portio));
        port_outb(lba, ATA_REG_LBA_LO(atadev->portio));
        port_outb(lba >> 8, ATA_REG_LBA_MID(atadev->portio));
        port_outb(lba >> 16, ATA_REG_LBA_HI(atadev->portio));

        /* Write extended command */
        port_outb(0x34, ATA_REG_COMMAND(atadev->portio));
        break;

    case ATA_TYPE_LBA28:
        port_outb(
            0xE0 | (!atadev->master << 4) | ((lba >> 24) & 0x0F),
            ATA_REG_DRIVE_SEL(atadev->portio));

        port_outb(sectors, ATA_REG_SECTOR(atadev->portio));

        port_outb(lba, ATA_REG_LBA_LO(atadev->portio));
        port_outb(lba >> 8, ATA_REG_LBA_MID(atadev->portio));
        port_outb(lba >> 16, ATA_REG_LBA_HI(atadev->portio));
        /* Write command. */
        port_outb(0x30, ATA_REG_COMMAND(atadev->portio));
        break;

    case ATA_TYPE_CHS:
        ASSERT_NOT_HIT(); // no

    default:
        KLOGF(ERR, "%s() - [%s] invalid ATA device!", __FUNCTION__, dev->name);
        return -EINVAL;
    }

    return sectors;
}

static bool atapio_flush_sectors(time_t timeout_ms, const BlockDevice* dev)
{
    const AtaStorageDevice* atadev = dev->extra;
    /* Send the actual command. */
    port_outb(0xE7, ATA_REG_COMMAND(atadev->portio));
    /* Wait for the sectors to actually flush. */
    Timer t;
    timer_start(&t);
    while (port_inb(ATA_REG_STATUS(atadev->portio)) & ATA_STATUS_BSY)
    {
        if (timer_elapsed_ms(&t) > timeout_ms)
        {
            KLOGF(
                ERR,
                "[%s] Timed out trying to flush sectors to disk!",
                dev->name);
            return false;
        }
    }

    return true;
}

static bool atapio_wait_for_ready(time_t timeout_ms, const BlockDevice* dev)
{
    const AtaStorageDevice* atadev = dev->extra;

    Timer t;
    timer_start(&t);
    while (true)
    {
        if (timer_elapsed_ms(&t) > timeout_ms)
        {
            KLOGF(
                ERR, "[%s] Timed out waiting for drive get ready!", dev->name);
            return false;
        }

        u8 status = port_inb(ATA_REG_STATUS(atadev->portio));
        if (!(status & ATA_STATUS_BSY) && (status & ATA_STATUS_DRQ))
            break;

        if ((status & ATA_STATUS_ERR) || (status & ATA_STATUS_DF))
        {
            /* If the ATA device uses LBA48, then the error register
            is 16bit, even if only 8 bits are used. Should I read a u16 ?
            does it care if I don't ? idk */
            u8 error = port_inb(ATA_REG_ERR(atadev->portio));

            KLOGF(ERR, "[%s] Error - %s!", dev->name, ata_error_to_str(error));
            return false;
        }
    }

    return true;
}

static _ATTR_ALWAYS_INLINE bool
atapio_is_valid_range(lba_t lba, sectorcnt_t sectors, const BlockDevice* dev)
{
    /* Check for possible underflow. */
    if (lba > dev->sector_count || sectors == 0)
        return false;

    return (sectors <= dev->sector_count - lba);
}

ssize_t
atapio_read(const BlockDevice* dev, lba_t lba, sectorcnt_t sectors, void* dest)
{
    if (!atapio_is_valid_range(lba, sectors, dev))
    {
        KLOGF(ERR, "[%s] Out of range read!", dev->name);
        return -EINVAL;
    }

    const AtaStorageDevice* atadev = dev->extra;

    while (sectors)
    {
        const u16 workingsectors = min(sectors, 256);

        int st = atapio_send_read_cmd(
            lba, atapio_internal_sectors(workingsectors), dev);

        if (st < 0)
            return st;

        for (size_t sector = 0; sector < workingsectors; ++sector)
        {
            /* If trying to read multiple sectors, sleep between reads */
            if (sector)
            {
                const struct timespec ts = {.tv_nsec = 400, .tv_sec = 0};
                nanosleep(&ts, NULL);
            }

            st = atapio_wait_for_ready(ATA_READ_TIMEOUT_MS, dev);
            if (st < 0)
                return st;

            for (size_t i = 0; i < 256; ++i)
            {
                /* We could cast buf to an u16* which would make
                the logic simpler, but that means we fuck up the
                alignment of buf. */
                u16 w = port_inw(ATA_REG_DATA(atadev->portio));
                ((u8*)dest)[i * 2] = w;
                ((u8*)dest)[i * 2 + 1] = (w >> 8) & 0xFF;
            }
        }

        sectors -= workingsectors;
        lba += workingsectors;
    }

    return sectors;
}

ssize_t atapio_write(
    const BlockDevice* dev, lba_t lba, sectorcnt_t sectors, const void* src)
{
    if (!atapio_is_valid_range(lba, sectors, dev))
    {
        KLOGF(ERR, "[%s] Out of range write!", dev->name);
        return -EINVAL;
    }

    const AtaStorageDevice* atadev = dev->extra;

    while (sectors)
    {
        const size_t workingsectors = min(sectors, 256);

        int st = atapio_send_write_cmd(
            lba, atapio_internal_sectors(workingsectors), dev);

        if (st < 0)
            return st;

        const struct timespec ts = {.tv_nsec = 400, .tv_sec = 0};
        for (size_t sector = 0; sector < workingsectors; ++sector)
        {
            if (sector)
                nanosleep(&ts, NULL);

            st = atapio_wait_for_ready(ATA_READ_TIMEOUT_MS, dev);
            if (st < 0)
                return st;

            for (u16 i = 0; i < 256; ++i)
            {
                /* We could cast buf to an u16* which would make
                the logic simpler, but that means we fuck up the
                alignment of buf. */
                u16 w = ((u8*)src)[i * 2 + 1];
                w <<= 8;
                w |= ((u8*)src)[i * 2];

                port_outw(w, ATA_REG_DATA(atadev->portio));

                /* Sleep just for good measure. ? */
                nanosleep(&ts, NULL);
            }
        }

        st = atapio_flush_sectors(ATA_FLUSH_SECTORS_TIMEOUT_MS, dev);
        if (st < 0)
            return st;

        sectors -= workingsectors;
        lba += workingsectors;
    }

    return sectors;
}
