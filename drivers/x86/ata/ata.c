/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include "ata.h"
#include <dxgmx/attrs.h>
#include <dxgmx/interrupts.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/module.h>
#include <dxgmx/storage/blkdevm.h>
#include <dxgmx/string.h>
#include <dxgmx/timer.h>
#include <dxgmx/todo.h>
#include <dxgmx/utils/bytes.h>
#include <dxgmx/x86/idt.h>
#include <dxgmx/x86/portio.h>

#define KLOGF_PREFIX "ata: "

static void ata_dump_device_info(const BlockDevice* dev)
{
    char unit[4] = {0};
    const char* mode;
    size_t size =
        bytes_to_human_readable(dev->sector_count * dev->sectorsize, unit);

    AtaStorageDevice* atadev = dev->extra;
    if (!atadev)
        return;

    switch (atadev->type)
    {
    case ATA_TYPE_LBA28:
        mode = "LBA28";
        break;
    case ATA_TYPE_LBA48:
        mode = "LBA48";
        break;
    case ATA_TYPE_CHS:
        mode = "CHS";
        break;
    default:
        mode = "???";
        break;
    }

    KLOGF(
        DEBUG,
        "[%s] %s (%u%s) on bus %X:%X mode %s.",
        dev->name,
        atadev->master ? "Master" : "Slave",
        size,
        unit,
        atadev->bus_io,
        atadev->bus_ctrl,
        mode);
}

static bool ata_generate_drive_name(BlockDevice* dev, BlockDeviceDriver* drv)
{
    char suffix = 'a';
    FOR_EACH_DRIVER_BLKDEV (drv, blkdev)
    {
        if (!blkdev->type || !blkdev->name)
            continue;

        if (strcmp(blkdev->type, "pata") == 0 && blkdev->name[2] >= suffix)
            suffix = blkdev->name[2] + 1;
    }

    if (suffix > 'z')
        return false; // for now

    dev->name = kmalloc(4);
    if (!dev->name)
        return false;

    strcpy(dev->name, "hda");
    dev->name[2] = suffix;

    return true;
}

static BlockDevice* ata_new_blkdev(BlockDeviceDriver* drv)
{
    BlockDevice* blkdev = blkdevdrv_new_blkdev(drv);
    if (!blkdev)
        return NULL;

    blkdev->extra = kcalloc(sizeof(AtaStorageDevice));
    if (!blkdev->extra)
    {
        blkdevdrv_free_blkdev(blkdev, drv);
        return NULL;
    }

    return blkdev;
}

static int ata_free_device(BlockDevice* blkdev, BlockDeviceDriver* drv)
{
    kfree(blkdev->extra);
    if (blkdev->name)
        kfree(blkdev->name);

    blkdevdrv_free_blkdev(blkdev, drv);
    return 0;
}

static bool ata_identify_drive(
    atabus_t bus_io, atabus_t bus_ctrl, bool master, BlockDeviceDriver* drv)
{
    /* We only care about PATA devices, other types will be ignored. */

    port_outw(0, ATA_SECTOR_REG(bus_io));
    port_outw(0, ATA_LBA_LO_REG(bus_io));
    port_outw(0, ATA_LBA_MID_REG(bus_io));
    port_outw(0, ATA_LBA_HI_REG(bus_io));

    /* send IDENTIFY command. */
    port_outb(0xEC, ATA_COMMAND_REG(bus_io));

    u8 status = port_inb(ATA_STATUS_REG(bus_io));
    if (!status)
        return false; /* doesn't exist */

    Timer timer;
    timer_start(&timer);

    /* wait until BSY bit clears. */
    while (port_inb(ATA_STATUS_REG(bus_io)) & ATAPIO_STATUS_BSY)
    {
        if (timer_elapsed_ms(&timer) > 200)
        {
            KLOGF(WARN, "Timed out waiting for BSY bit to clear.");
            return false;
        }
    }

    u8 lbamid = port_inb(ATA_LBA_MID_REG(bus_io));
    u8 lbahi = port_inb(ATA_LBA_HI_REG(bus_io));

    if (lbamid || lbahi)
        return false; /* Not ATAPIO device. */

    timer_start(&timer);
    /* Wait until either ERR or DRQ set. */
    while (true)
    {
        if (timer_elapsed_ms(&timer) > 200)
        {
            KLOGF(WARN, "Timed out waiting to ERR/DRQ bits.");
            return false;
        }

        status = port_inb(ATA_STATUS_REG(bus_io));
        if (status & ATAPIO_STATUS_ERR)
        {
            KLOGF(
                ERR,
                "Failed to identify drive (%s, %u:%u)!",
                master ? "master" : "slave",
                bus_io,
                bus_ctrl);
            return false;
        }

        if (status & ATAPIO_STATUS_DRQ)
            break;
    }

    BlockDevice* dev = ata_new_blkdev(drv);
    if (!dev)
    {
        for (size_t i = 0; i < 256; ++i)
            port_inw(ATA_DATA_REG(bus_io));

        KLOGF(ERR, "Failed to allocate a new device!");
        return false;
    }

    if (!ata_generate_drive_name(dev, drv))
    {
        ata_free_device(dev, drv);
        // FIXME: we should delete the allocated 'dev'.
        for (size_t i = 0; i < 256; ++i)
            port_inw(ATA_DATA_REG(bus_io));

        KLOGF(ERR, "Failed to generate drive name!");
        return false;
    }

    AtaStorageDevice* atadev = dev->extra;
    dev->type = "pata";
    dev->sectorsize = 512;
    atadev->master = master;
    atadev->bus_io = bus_io;
    atadev->bus_ctrl = bus_ctrl;
    atadev->type = ATA_TYPE_UNKNOWN;

    /* Read a whole sector containing information about the drive. */
    for (u16 i = 0; i < 256; ++i)
    {
        u16 val = port_inw(ATA_DATA_REG(bus_io));

        switch (i)
        {
        case 60:
            dev->sector_count = val;
            dev->sector_count |= (port_inw(ATA_DATA_REG(bus_io)) << 16);
            ++i;
            break;

        case 83:
            if ((val & (1 << 10)))
                atadev->type = ATA_TYPE_LBA48;
            break;

        case 88:
            break;

        case 93:
            atadev->bigcable = (bool)(val & (1 << 11));
            break;

        case 100:
            if (atadev->type == ATA_TYPE_LBA48)
            {
                dev->sector_count = 0;
                const u32 tmp = val | (port_inw(ATA_DATA_REG(bus_io)) << 16);
                dev->sector_count |= (port_inw(ATA_DATA_REG(bus_io)));
                dev->sector_count |= (port_inw(ATA_DATA_REG(bus_io)) << 16);
                dev->sector_count <<= 32;
                dev->sector_count |= tmp;

                i += 3;
            }
            break;

        default:
            break;
        }
    }

    /* Disable interrupts */
    port_outb(
        port_inb(ATA_DEV_CTRL_REG(bus_ctrl)) & ~(2),
        ATA_DEV_CTRL_REG(bus_ctrl));

    /* Do LBA48 drives support LBA28 ? */
    if (atadev->type == ATA_TYPE_UNKNOWN)
        atadev->type = ATA_TYPE_LBA28;

    if (atadev->dma)
    {
        TODO();
    }
    else
    {
        dev->read = atapio_read;
        dev->write = atapio_write;
    }

    ata_dump_device_info(dev);
    blkdevm_enumerate_partitions(dev);

    return true;
}

static u8
ata_identify_bus(atabus_t bus_io, atabus_t bus_ctrl, BlockDeviceDriver* drv)
{
    port_outb(0, ATA_DEV_CTRL_REG(bus_ctrl));

    u8 ret = 0;

    /* try master drive. */
    port_outb(0xA0, ATA_DRIVE_SEL_REG(bus_io));
    ret += ata_identify_drive(bus_io, bus_ctrl, true, drv);

    /* try slave drive. */
    port_outb(0xB0, ATA_DRIVE_SEL_REG(bus_io));
    ret += ata_identify_drive(bus_io, bus_ctrl, false, drv);

    return ret;
}

static void ata_primary_isr()
{
    interrupts_irq_done();
}

static void ata_secondary_isr()
{
    interrupts_irq_done();
}

static int ata_init(BlockDeviceDriver* drv)
{
    interrupts_reqister_irq_isr(IRQ_ATA1, ata_primary_isr);
    interrupts_reqister_irq_isr(IRQ_ATA2, ata_secondary_isr);

    ata_identify_bus(0x1F0, 0x3F6, drv);

    ata_identify_bus(0x170, 0x376, drv);

    return 0;
}

static int ata_destroy(BlockDeviceDriver* drv)
{
    (void)drv;
    return 0;
}

static BlockDeviceDriver g_ata_driver = {
    .init = ata_init, .destroy = ata_destroy};

static int ata_main()
{
    blkdevm_register_blkdev_driver(&g_ata_driver);
    return 0;
}

static int ata_exit()
{
    blkdevm_unregister_blkdev_driver(&g_ata_driver);

    return 0;
}

MODULE g_ata_module = {
    .name = "ata", .main = ata_main, .exit = ata_exit, .stage = MODULE_STAGE3};
