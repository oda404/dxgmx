/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/x86/ata.h>
#include<dxgmx/x86/portio.h>
#include<dxgmx/x86/idt.h>
#include<dxgmx/x86/pci.h>
#include<dxgmx/klog.h>
#include<dxgmx/attrs.h>
#include<dxgmx/kmalloc.h>
#include<dxgmx/string.h>
#include<dxgmx/panic.h>
#include<dxgmx/assert.h>
#include<dxgmx/time.h>
#include<dxgmx/todo.h>
#include<dxgmx/timer.h>
#include<dxgmx/utils/bytes.h>

#define KLOGF(lvl, fmt, ...) klogln(lvl, "ata: " fmt, ##__VA_ARGS__)

#define ATA_DEVICE_CTRL_PORT 0x3F6
#define ATA_ALTERNATE_STATUS_PORT 0x376

#define ATAPIO_READ 0x20
#define ATAPIO_WRITE 0x30
#define ATAPIO_READ_EXT 0x24
#define ATAPIO_WRITE_EXT 0x34
#define ATAPIO_FLUSH_SECTOR 0xE7

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

static size_t g_ata_devices_count = 0;
static ATADevice *g_ata_devices = NULL;

static void ata_dump_device_info(const ATADevice *dev)
{
    char unit[4] = { 0 };
    const char *mode;
    size_t size = bytes_to_human_readable(dev->sector_count * ATA_DISK_SECTOR_SIZE, unit);
    
    if(dev->lba28)
        mode = "LBA28";
    else if(dev->lba48)
        mode = "LBA48";
    else
        mode = "CHS";

    KLOGF(
        DEBUG, 
        "%s device (%u%s) on bus %X:%X mode %s.",
        dev->master ? "Master" : "Slave",
        size, unit,
        dev->bus_io, dev->bus_ctrl,
        mode
    );
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
    if(!status)
        return false; /* doesn't exist */

    /* wait until BSY bit clears. */
    while(port_inb(ATA_STATUS_REG(bus_io)) & ATAPIO_STATUS_BSY);

    u8 lbamid = port_inb(ATA_LBA_MID_REG(bus_io));
    u8 lbahi = port_inb(ATA_LBA_HI_REG(bus_io));

    if(lbamid || lbahi)
        return false; /* Not ATAPIO device. */

    /* Wait until either ERR or DRQ set. */
    while(true)
    {
        status = port_inb(ATA_STATUS_REG(bus_io));
        if(status & ATAPIO_STATUS_ERR)
            return false;

        if(status & ATAPIO_STATUS_DRQ)
            break;
    }

    g_ata_devices = krealloc(g_ata_devices, (++g_ata_devices_count) * sizeof(ATADevice));
    ATADevice *dev = &g_ata_devices[g_ata_devices_count - 1];
    memset(dev, 0, sizeof(ATADevice));

    dev->master = master;
    dev->bus_io = bus_io;
    dev->bus_ctrl = bus_ctrl;

    /* Read a whole sector containing information about the drive. */
    for(u16 i = 0; i < 256; ++i)
    {
        u16 val = port_inw(ATA_DATA_REG(bus_io));

        switch(i)
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
            if(dev->lba48)
            {
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

    /* Do LBA48 drives support LBA28 ? */
    dev->lba28 = (!dev->lba48 && !dev->chs);

    if(dev->dma)
    {
        TODO();
    }
    else
    {
        dev->read_func = atapio_read;
        dev->write_func = atapio_write;
    }

    /* Disable interrupts */
    port_outb(
        port_inb(ATA_DEV_CTRL_REG(bus_ctrl)) & ~(2), 
        ATA_DEV_CTRL_REG(bus_ctrl)
    );

    return true;
}

static u8 ata_identify_bus(u16 bus_io, u16 bus_ctrl)
{
    port_outb(0, ATA_DEV_CTRL_REG(bus_ctrl));\

    u8 ret = 0;

    /* try master drive. */
    port_outb(0xA0, ATA_DRIVE_SEL_REG(bus_io));
    ret += ata_identify_drive(bus_io, bus_ctrl, true);
    
    /* try slave drive. */
    port_outb(0xB0, ATA_DRIVE_SEL_REG(bus_io));
    ret += ata_identify_drive(bus_io, bus_ctrl, false);

    return ret;
}

static void ata_primary_isr(const InterruptFrame *frame, const void *)
{
    (void)frame;
}

static void ata_secondary_isr(const InterruptFrame *frame, const void *)
{
    (void)frame;
}

_INIT int ata_init()
{
    idt_register_isr(IRQ14, ata_primary_isr);
    idt_register_isr(IRQ15, ata_secondary_isr);

    ata_identify_bus(0x1F0, 0x3F6);

    ata_identify_bus(0x170, 0x376);

    for(size_t i = 0; i < g_ata_devices_count; ++i)
        ata_dump_device_info(&g_ata_devices[i]);

    return 0;
}

const ATADevice *ata_get_devices(size_t *count)
{
    if(count)
        *count = g_ata_devices_count;
    return g_ata_devices;
}

bool ata_read(u64 start, u16 sectors, u8 *buf, const ATADevice *dev)
{
    if(dev && dev->read_func)
        return dev->read_func(start, sectors, buf, dev);

    return false;
}

bool ata_write(u64 start, u16 sectors, const u8* buf, const ATADevice *dev)
{
    if(dev && dev->write_func)
        return dev->write_func(start, sectors, buf, dev);

    return false;
}

bool atapio_read(u64 lba, u16 sectors, u8 *buf, const ATADevice *dev)
{
    if(!dev || !buf)
        return false;

    size_t actualsectors = 0;

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

        actualsectors = sectors == 0 ? 65536 : sectors;
    }
    else if(dev->lba28)
    {
        port_outb(0xE0 | (!dev->master << 4) | ((lba >> 24) & 0x0F), ATA_DRIVE_SEL_REG(dev->bus_io));

        port_outb(sectors, ATA_SECTOR_REG(dev->bus_io));

        port_outb(lba, ATA_LBA_LO_REG(dev->bus_io));
        port_outb(lba >> 8, ATA_LBA_MID_REG(dev->bus_io));
        port_outb(lba >> 16, ATA_LBA_HI_REG(dev->bus_io));

        port_outb(ATAPIO_READ, ATA_COMMAND_REG(dev->bus_io));

        actualsectors = sectors == 0 ? 256 : sectors;
    }
    else
    {
        KLOGF(ERR, "atapio_read() called with non-PIO drive!");
        return false;
    }
    
    /* The rest applies to both lba28 and 48. */
#define ATAPIO_READ_TIMEOUT_MS 200
    
    struct timespec ts = { .tv_nsec = 400, .tv_sec = 0 };
    for(size_t i = 0; i < actualsectors; ++i)
    {
        if(i > 0)
            nanosleep(&ts, NULL);
            
        Timer t;
        timer_start(&t);

        while(true)
        {
            if(timer_ellapsed_ms(&t) > ATAPIO_READ_TIMEOUT_MS)
            {
                KLOGF(ERR, "Timed out trying to read from disk!");
                return false;
            }

            u8 status = port_inb(ATA_STATUS_REG(dev->bus_io));
            if( !(status & ATAPIO_STATUS_BSY) && (status & ATAPIO_STATUS_DRQ) )
                break;

            if( (status & ATAPIO_STATUS_ERR) || (status & ATAPIO_STATUS_DF) )
            {
                KLOGF(WARN, "Error trying to read from disk.");
                return false;
            }
        }
        
        for(size_t k = 0; k < 256; ++k)
            ((u16 *)buf)[i * 256 + k] = port_inw(ATA_DATA_REG(dev->bus_io));
    }

    return true;
}

bool atapio_write(u64 lba, u16 sectors, const u8* buf, const ATADevice *dev)
{
    if(!dev || !buf)
        return 0;

    size_t actualsectors = 0;

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

        actualsectors = sectors == 0 ? 65536 : sectors;
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

        actualsectors = sectors == 0 ? 256 : sectors;
    }
    else
    {
        KLOGF(ERR, "atapio_write() called with non-PIO drive!");
        return false;
    }

#define ATAPIO_WRITE_TIMEOUT_MS 200

    struct timespec ts = { .tv_nsec = 400, .tv_sec = 0 };
    for(size_t i = 0; i < actualsectors; ++i)
    {
        if(i > 0)
            nanosleep(&ts, NULL);

        Timer t;
        timer_start(&t);
        while(true)
        {
            if(timer_ellapsed_ms(&t) > ATAPIO_WRITE_TIMEOUT_MS)
            {
                KLOGF(ERR, "Timed out trying to write from disk!");
                return false;
            }

            u8 status = port_inb(ATA_STATUS_REG(dev->bus_io));
            if( !(status & ATAPIO_STATUS_BSY) && (status & ATAPIO_STATUS_DRQ) )
                break;

            if( (status & ATAPIO_STATUS_ERR) || (status & ATAPIO_STATUS_DF) )
            {
                KLOGF(WARN, "error trying to write to disk.");
                return false;
            }
        }

        for(size_t k = 0; k < actualsectors * 256; ++k)
        {
            port_outw(((const u16 *)buf)[i * 256 + k], ATA_DATA_REG(dev->bus_io));
            /* Sleep 400ns just for good measure.  */
            nanosleep(&ts, NULL);
        }

        port_outb(ATAPIO_FLUSH_SECTOR, ATA_COMMAND_REG(dev->bus_io));
        /* Wait for the sectors to actually flush. */
        timer_start(&t);
        while(port_inb(ATA_STATUS_REG(dev->bus_io)) & ATAPIO_STATUS_BSY)
        {
            if(timer_ellapsed_ms(&t) > ATAPIO_WRITE_TIMEOUT_MS)
            {
                KLOGF(ERR, "Timed out trying to flush sectors to disk!");
                return false;
            }
        }
    }

    return 0;
}
