/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include "ata.h"
#include <dxgmx/attrs.h>
#include <dxgmx/errno.h>
#include <dxgmx/interrupts.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/mem/dma.h>
#include <dxgmx/mem/falloc.h>
#include <dxgmx/mem/mm.h>
#include <dxgmx/module.h>
#include <dxgmx/pci.h>
#include <dxgmx/storage/blkdevm.h>
#include <dxgmx/string.h>
#include <dxgmx/timer.h>
#include <dxgmx/todo.h>
#include <dxgmx/utils/bytes.h>
#include <dxgmx/x86/idt.h>
#include <dxgmx/x86/portio.h>

#define KLOGF_PREFIX "ide: "

static BlockDeviceDriver g_ata_driver = {.init = NULL, .destroy = NULL};

static void ata_primary_isr()
{
    interrupts_irq_done();
}

static void ata_secondary_isr()
{
    interrupts_irq_done();
}

const char* ata_error_to_str(u8 error)
{
    switch (error)
    {
    case ATA_ERR_AMNF:
        return "Address mark not found";
    case ATA_ERR_TKZNF:
        return "Track zero not found";
    case ATA_ERR_ABRT:
        return "Aborted command";
    case ATA_ERR_MCR:
        return "Media change request";
    case ATA_ERR_IDNF:
        return "ID not found";
    case ATA_ERR_MC:
        return "Media changed";
    case ATA_ERR_UNC:
        return "Uncorrectable data error";
    case ATA_ERR_BBK:
        return "Bad block detected";
    default:
        return "Unknown error";
    }
}

static void ide_dump_drive_info(const BlockDevice* dev)
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
        "[%s] %s (%u%s) mode %s.",
        dev->name,
        atadev->master ? "Master" : "Slave",
        size,
        unit,
        mode);
}

static bool ide_generate_drive_name(BlockDevice* dev, BlockDeviceDriver* drv)
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

static BlockDevice* ide_new_blkdev(BlockDeviceDriver* drv)
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

static int ide_free_blkdev(BlockDevice* blkdev, BlockDeviceDriver* drv)
{
    kfree(blkdev->extra);
    if (blkdev->name)
        kfree(blkdev->name);

    blkdevdrv_free_blkdev(blkdev, drv);
    return 0;
}

static void ide_select_drive(bool master, u16 bus_io)
{
    port_outb(master ? 0xA0 : 0xB0, ATA_REG_DRIVE_SEL(bus_io));
    sleep_ms(1);
}

static int ide_identify_drive(
    u16 bus_io,
    u16 bus_ctl,
    PCIDevice* pcidev,
    bool master,
    BlockDeviceDriver* drv)
{
    /* Select drive */
    ide_select_drive(master, bus_io);

    /* OSdev says to check for 0 status AFTER we send the identify command, but
     * I get a dummy drive if I don't check for it here. */
    if (port_inb(ATA_REG_STATUS(bus_io)) == 0)
        return -ENODEV;

    /* Zero out lba and sector */
    port_outw(0, ATA_REG_SECTOR(bus_io));
    port_outw(0, ATA_REG_LBA_LO(bus_io));
    port_outw(0, ATA_REG_LBA_MID(bus_io));
    port_outw(0, ATA_REG_LBA_HI(bus_io));

    /* send IDENTIFY command. */
    port_outb(ATA_CMD_IDENT, ATA_REG_COMMAND(bus_io));

    u8 status = port_inb(ATA_REG_STATUS(bus_io));
    if (!status)
        return -ENODEV; /* doesn't exist */

    /* Check for SATA/ATAPI. SATA/ATAPI set error after an identify */
    {
        const u8 a = port_inb(ATA_REG_LBA_MID(bus_io));
        const u8 b = port_inb(ATA_REG_LBA_HI(bus_io));
        if (a == 0x14 && b == 0xEB)
            return -EINVAL; // ATAPI, we should support this

        if (a == 0x69 && b == 0x96)
            return -EINVAL; // Also ATAPI ?

        if (a == 0x3c && b == 0xc3)
            return -EINVAL; // SATA, we should support this.
    }

    Timer timer;
    /* wait until BSY bit clears. */
    timer_start(&timer);
    while (port_inb(ATA_REG_STATUS(bus_io)) & ATA_STATUS_BSY)
    {
        if (timer_elapsed_ms(&timer) > 200)
        {
            KLOGF(WARN, "Timed out waiting for BSY bit to clear.");
            return -ETIMEDOUT;
        }
    }

    {
        /* As per osdev advice, if they are not zero they are not ATA */
        const u8 lbamid = port_inb(ATA_REG_LBA_MID(bus_io));
        const u8 lbahi = port_inb(ATA_REG_LBA_HI(bus_io));
        if (lbamid || lbahi)
            return -EINVAL; /* Not PATA device. */
    }

    /* Wait until either ERR or DRQ set. */
    timer_start(&timer);
    while (true)
    {
        if (timer_elapsed_ms(&timer) > 200)
        {
            KLOGF(WARN, "Timed out waiting for ERR/DRQ bits.");
            return -ETIMEDOUT;
        }

        status = port_inb(ATA_REG_STATUS(bus_io));
        if (status & ATA_STATUS_ERR)
        {
            u8 err = port_inb(ATA_REG_ERR(bus_io));
            KLOGF(ERR, "%s", ata_error_to_str(err));
            return -EIO;
        }

        if (status & ATA_STATUS_DRQ)
            break;
    }

    BlockDevice* dev = ide_new_blkdev(drv);
    if (!dev)
    {
        for (size_t i = 0; i < 256; ++i)
            port_inw(ATA_REG_DATA(bus_io));

        return -ENOMEM;
    }

    if (!ide_generate_drive_name(dev, drv))
    {
        ide_free_blkdev(dev, drv);
        for (size_t i = 0; i < 256; ++i)
            port_inw(ATA_REG_DATA(bus_io));

        return -ENOMEM;
    }

    AtaStorageDevice* atadev = dev->extra;
    dev->type = "pata";
    dev->sectorsize = 512;
    atadev->master = master;
    atadev->portio = bus_io;
    atadev->portctl = bus_ctl;
    atadev->type = ATA_TYPE_UNKNOWN;

    /* Read a whole sector containing information about the drive. */
    for (u16 i = 0; i < 256; ++i)
    {
        u16 val = port_inw(ATA_REG_DATA(bus_io));

        switch (i)
        {
        case 60:
            dev->sector_count = val;
            dev->sector_count |= (port_inw(ATA_REG_DATA(bus_io)) << 16);
            ++i;
            break;

        case 83:
            if ((val & (1 << 10)))
                atadev->type = ATA_TYPE_LBA48;
            break;

        case 88:
            atadev->dma = (bool)(val >> 8) & 0xFF;
            break;

        case 93:
            // atadev->bigcable = (bool)(val & (1 << 11));
            break;

        case 100:
            if (atadev->type == ATA_TYPE_LBA48)
            {
                dev->sector_count = 0;
                const u32 tmp = val | (port_inw(ATA_REG_DATA(bus_io)) << 16);
                dev->sector_count |= (port_inw(ATA_REG_DATA(bus_io)));
                dev->sector_count |= (port_inw(ATA_REG_DATA(bus_io)) << 16);
                dev->sector_count <<= 32;
                dev->sector_count |= tmp;

                i += 3;
            }
            break;

        default:
            break;
        }
    }

    if (atadev->type == ATA_TYPE_UNKNOWN)
        atadev->type = ATA_TYPE_LBA28;

    if (atadev->dma)
    {
        TODO();
        dev->read = atapio_read;
        dev->write = atapio_write;
    }
    else
    {
        dev->read = atapio_read;
        dev->write = atapio_write;
    }

    atadev->controller = pcidev;

    ide_dump_drive_info(dev);
    blkdevm_enumerate_partitions(dev);

    return true;
}

static int ide_enumerate_bus(
    u16 bus_io, u16 bus_ctl, PCIDevice* dev, BlockDeviceDriver* drv)
{
    /* try master drive. */
    ide_identify_drive(bus_io, bus_ctl, dev, true, drv);

    /* try slave drive. */
    ide_identify_drive(bus_io, bus_ctl, dev, false, drv);

    return 0;
}

/* Called by the PCI subsystem if there is a device matching our driver. */
static int ide_probe_pci_device(PCIDevice* dev)
{
    /* Compatibility ports */
    u16 primary_bus_io = 0x1F0;
    u16 primary_bus_ctl = 0x3F6;
    u8 primary_interrupt_line = IRQ_ATA1;

    /* If bit 0 is set, the primary ata bus is in PCI native mode and we should
     * read BARs to get the bus_io/ctl/intline */
    if (dev->progif & (1 << 0))
    {
    }

    /* Compatibility ports */
    u16 secondary_bus_io = 0x170;
    u16 secondary_bus_ctl = 0x376;
    u8 secondary_interrupt_line = IRQ_ATA2;

    /* If bit 2 is set, the secondary ata bus is in PCI native mode and we
     * should read BARs to get the bus_io/ctl/intline */
    if (dev->progif & (1 << 2))
    {
    }

    interrupts_reqister_irq_isr(primary_interrupt_line, ata_primary_isr);
    interrupts_reqister_irq_isr(secondary_interrupt_line, ata_secondary_isr);

    ide_enumerate_bus(primary_bus_io, primary_bus_ctl, dev, &g_ata_driver);
    ide_enumerate_bus(secondary_bus_io, secondary_bus_ctl, dev, &g_ata_driver);

    return 0;
}

static int ide_main()
{
    static const PCIDeviceDriver driver = {
        .name = "ide",
        .class = PCI_MASS_STORAGE_CONTROLLER,
        .subclass = 1,
        .probe = ide_probe_pci_device};

    int st = pci_register_device_driver(&driver);
    if (st < 0)
        return st;

    st = blkdevm_register_blkdev_driver(&g_ata_driver);
    if (st < 0)
    {
        pci_unregister_device_driver(&driver);
        return st;
    }
    return 0;
}

static int ide_exit()
{
    blkdevm_unregister_blkdev_driver(&g_ata_driver);
    return 0;
}

MODULE g_ata_module = {
    .name = "ata", .main = ide_main, .exit = ide_exit, .stage = MODULE_STAGE3};
