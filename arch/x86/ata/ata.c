/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/string.h>
#include <dxgmx/todo.h>
#include <dxgmx/utils/bytes.h>
#include <dxgmx/vfs.h>
#include <dxgmx/x86/ata.h>
#include <dxgmx/x86/idt.h>
#include <dxgmx/x86/portio.h>

#define KLOGF(lvl, fmt, ...) klogln(lvl, "ata: " fmt, ##__VA_ARGS__)

static GenericStorageDevice* g_ata_devices = NULL;
static size_t g_ata_devices_count = 0;

static void ata_dump_device_info(const GenericStorageDevice* dev)
{
    char unit[4] = {0};
    const char* mode;
    size_t size = bytes_to_human_readable(
        dev->sectors_count * dev->physical_sectorsize, unit);

    AtaStorageDevice* atadev = dev->extra;

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

static bool ata_generate_drive_name(GenericStorageDevice* dev)
{
    if (!dev)
        return false;

    if (g_ata_devices_count > 26)
        return false; // for now

    dev->name = kmalloc(4);
    if (!dev->name)
        return false;

    strcpy(dev->name, "hda");
    dev->name[2] += g_ata_devices_count - 1;

    return true;
}

static GenericStorageDevice* ata_new_device()
{
    GenericStorageDevice dev;
    memset(&dev, 0, sizeof(dev));
    /* Allocate the AtaStorageDevice struct */
    if (!(dev.extra = kmalloc(sizeof(AtaStorageDevice))))
        return NULL;

    /* Try to enlarge the devices table */
    GenericStorageDevice* tmp = krealloc(
        g_ata_devices,
        (g_ata_devices_count + 1) * sizeof(GenericStorageDevice));

    if (!tmp)
    {
        kfree(dev.extra);
        return NULL;
    }

    /* If we got here everything is fine. */
    memset(dev.extra, 0, sizeof(AtaStorageDevice));
    ++g_ata_devices_count;
    g_ata_devices = tmp;

    tmp = &g_ata_devices[g_ata_devices_count - 1];
    *tmp = dev;
    return tmp;
}

static bool ata_identify_drive(atabus_t bus_io, atabus_t bus_ctrl, bool master)
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

    /* wait until BSY bit clears. */
    while (port_inb(ATA_STATUS_REG(bus_io)) & ATAPIO_STATUS_BSY)
        ;

    u8 lbamid = port_inb(ATA_LBA_MID_REG(bus_io));
    u8 lbahi = port_inb(ATA_LBA_HI_REG(bus_io));

    if (lbamid || lbahi)
        return false; /* Not ATAPIO device. */

    /* Wait until either ERR or DRQ set. */
    while (true)
    {
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

    GenericStorageDevice* dev = ata_new_device();
    if (!dev)
    {
        for (size_t i = 0; i < 256; ++i)
            port_inw(ATA_DATA_REG(bus_io));

        KLOGF(ERR, "Failed to allocate a new device!");
        return false;
    }
    AtaStorageDevice* atadev = dev->extra;

    dev->type = "pata";
    dev->physical_sectorsize = 512;
    dev->logical_sectorsize = 512;
    atadev->master = master;
    atadev->bus_io = bus_io;
    atadev->bus_ctrl = bus_ctrl;
    atadev->type = ATA_TYPE_UNKNOWN;

    if (!ata_generate_drive_name(dev))
    {
        // FIXME: we should delete the allocated 'dev'.
        for (size_t i = 0; i < 256; ++i)
            port_inw(ATA_DATA_REG(bus_io));

        KLOGF(ERR, "Failed to generate drive name!");
        return false;
    }

    /* Read a whole sector containing information about the drive. */
    for (u16 i = 0; i < 256; ++i)
    {
        u16 val = port_inw(ATA_DATA_REG(bus_io));

        switch (i)
        {
        case 60:
            dev->sectors_count = val;
            dev->sectors_count |= (port_inw(ATA_DATA_REG(bus_io)) << 16);
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
                dev->sectors_count = 0;
                const u32 tmp = val | (port_inw(ATA_DATA_REG(bus_io)) << 16);
                dev->sectors_count |= (port_inw(ATA_DATA_REG(bus_io)));
                dev->sectors_count |= (port_inw(ATA_DATA_REG(bus_io)) << 16);
                dev->sectors_count <<= 32;
                dev->sectors_count |= tmp;

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

    return true;
}

static u8 ata_identify_bus(atabus_t bus_io, atabus_t bus_ctrl)
{
    port_outb(0, ATA_DEV_CTRL_REG(bus_ctrl));

    u8 ret = 0;

    /* try master drive. */
    port_outb(0xA0, ATA_DRIVE_SEL_REG(bus_io));
    ret += ata_identify_drive(bus_io, bus_ctrl, true);

    /* try slave drive. */
    port_outb(0xB0, ATA_DRIVE_SEL_REG(bus_io));
    ret += ata_identify_drive(bus_io, bus_ctrl, false);

    return ret;
}

static void ata_primary_isr(const InterruptFrame* frame, const void*)
{
    (void)frame;
}

static void ata_secondary_isr(const InterruptFrame* frame, const void*)
{
    (void)frame;
}

_INIT int ata_init()
{
    idt_register_isr(IRQ14, ata_primary_isr);
    idt_register_isr(IRQ15, ata_secondary_isr);

    ata_identify_bus(0x1F0, 0x3F6);

    ata_identify_bus(0x170, 0x376);

    for (size_t i = 0; i < g_ata_devices_count; ++i)
        ata_dump_device_info(&g_ata_devices[i]);

    /* Hardcoded at 1 because the bus reports 2 identical drives
    (master & slave) even though only one is being emulated ??? */
    for (size_t i = 0; i < 1; ++i)
        vfs_add_drive(&g_ata_devices[i]);

    return 0;
}

bool ata_read(
    lba_t lba, sector_t sectors, void* buf, const GenericStorageDevice* dev)
{
    if (dev && dev->read)
        return dev->read(lba, sectors, buf, dev);

    return false;
}

bool ata_write(
    lba_t lba,
    sector_t sectors,
    const void* buf,
    const GenericStorageDevice* dev)
{
    if (dev && dev->write)
        return dev->write(lba, sectors, buf, dev);

    return false;
}
