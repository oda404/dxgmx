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

static size_t g_ata_devices_count = 0;
static ATADevice* g_ata_devices = NULL;
#define ATA_DISK_SECTOR_SIZE 512
#define ATA_DISK_MAX_WRITE_SECTORS 256
#define ATA_DISK_MAX_READ_SECTORS 256

static void ata_dump_device_info(const ATADevice* dev)
{
    char unit[4] = {0};
    const char* mode;
    size_t size =
        bytes_to_human_readable(dev->sector_count * ATA_DISK_SECTOR_SIZE, unit);

    if (dev->lba28)
        mode = "LBA28";
    else if (dev->lba48)
        mode = "LBA48";
    else
        mode = "CHS";

    KLOGF(
        DEBUG,
        "[%s] %s (%u%s) on bus %X:%X mode %s.",
        dev->name,
        dev->master ? "Master" : "Slave",
        size,
        unit,
        dev->bus_io,
        dev->bus_ctrl,
        mode);
}

static bool ata_generate_drive_name(ATADevice* dev)
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

/* Identifies a previously selected device. */
static bool ata_identify_drive(size_t bus_io, size_t bus_ctrl, bool master)
{
    port_outw(0, ATA_SECTOR_REG(bus_io));
    port_outw(0, ATA_LBA_LO_REG(bus_io));
    port_outw(0, ATA_LBA_MID_REG(bus_io));
    port_outw(0, ATA_LBA_HI_REG(bus_io));

    /* send IDENTIFY command. */
    port_outb(ATA_IDENTIFY, ATA_COMMAND_REG(bus_io));

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
            return false;

        if (status & ATAPIO_STATUS_DRQ)
            break;
    }

    g_ata_devices =
        krealloc(g_ata_devices, (++g_ata_devices_count) * sizeof(ATADevice));
    ATADevice* dev = &g_ata_devices[g_ata_devices_count - 1];
    memset(dev, 0, sizeof(ATADevice));

    dev->master = master;
    dev->bus_io = bus_io;
    dev->bus_ctrl = bus_ctrl;

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
            dev->lba48 |= (bool)(val & (1 << 10));
            break;

        case 88:
            break;

        case 93:
            dev->bigcable = (bool)(val & (1 << 11));
            break;

        case 100:
            if (dev->lba48)
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
    dev->lba28 = (!dev->lba48 && !dev->chs);

    if (dev->dma)
    {
        TODO();
    }
    else
    {
        dev->read = (storage_drive_read)atapio_read;
        dev->write = (storage_drive_write)atapio_write;
    }

    if (!ata_generate_drive_name(dev))
    {
        KLOGF(ERR, "Failed to generate drive name!");
        return false;
    }

    return true;
}

static u8 ata_identify_bus(u16 bus_io, u16 bus_ctrl)
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
    (master & slave) even though only one being emulated ??? */
    for (size_t i = 0; i < 1; ++i)
    {
        ATADevice* dev = &g_ata_devices[i];

        GenericDrive drive = {
            .name = dev->name,
            .internal_dev = dev,
            .read = dev->read,
            .write = dev->write,
            .physical_sectorsize = 512,
        };

        vfs_add_drive(&drive);
    }

    return 0;
}

const ATADevice* ata_get_devices(size_t* count)
{
    if (count)
        *count = g_ata_devices_count;
    return g_ata_devices;
}

bool ata_read(u64 start, size_t n, u8* buf, const ATADevice* dev)
{
    if (dev && dev->read)
        return dev->read(start, n, buf, dev);

    return false;
}

bool ata_write(u64 start, size_t n, const u8* buf, const ATADevice* dev)
{
    if (dev && dev->write)
        return dev->write(start, n, buf, dev);

    return false;
}
