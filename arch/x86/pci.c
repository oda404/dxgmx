/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/errno.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/panic.h>
#include <dxgmx/x86/pci.h>
#include <dxgmx/x86/portio.h>

#define KLOGF_PREFIX "pci: "

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

static PCIDevice* g_pci_devices = NULL;
static size_t g_pci_devices_count = 0;

typedef union U_PCIDeviceRegister
{
    u32 whole;
    _ATTR_PACKED struct
    {
        u8 register_offset;
        u8 func : 3;
        u8 dev : 5;
        u8 bus;
        u8 reserved : 7;
        u8 enabled : 1;
    };
} PCIDeviceRegister;

static const char* pci_class_to_string(u8 class, u8 subclass)
{
    switch (class)
    {
    case PCI_MASS_STORAGE_CONTROLLER:
        switch (subclass)
        {
        case 0:
            return "SCSI Bus controller";
        case 1:
            return "IDE controller";
        case 2:
            return "Floppy Disk controller";
        case 3:
            return "IPI Bus controller";
        case 4:
            return "RAID controller";
        case 5:
            return "ATA controller";
        case 6:
            return "SATA controller";
        case 7:
            return "Serial Attached SCSI controller";
        case 8:
            return "NVM controller";
        case 0x80:
            return "Generic mass storage controller";
        default:
            return "Unknown mass storage controller";
        }

    case PCI_NETWORK_CONTROLLER:
        switch (subclass)
        {
        case 0:
            return "Ethernet controller";
        case 1:
            return "Token Ring controller";
        case 2:
            return "FDDI controller";
        case 3:
            return "ATM controller";
        case 4:
            return "ISDN controller";
        case 5:
            return "WorldFip controller";
        case 6:
            return "PICMG controller";
        case 7:
            return "Infiniband controller";
        case 8:
            return "Fabric controller";
        case 0x80:
            return "Generic network controller";
        default:
            return "Unknown network controller";
        }

    case PCI_DISPLAY_CONTROLLER:
        switch (subclass)
        {
        case 0:
            return "VGA Compatible controller";
        case 1:
            return "XGA Compatible controller";
        case 2:
            return "3D controller";
        case 0x80:
            return "Generic display controller";
        default:
            return "Unknown display controller";
        }

    case PCI_MULTIMEDIA_CONTROLLER:
        switch (subclass)
        {
        case 0:
            return "Multimedia video controller";
        case 1:
            return "Multimedia audio controller";
        case 2:
            return "Computer telephony controller";
        case 3:
            return "Audio device";
        case 0x80:
            return "Generic multimedia controller";
        default:
            return "Unknown multimedia controller";
        }

    case PCI_MEMORY_CONTROLLER:
        switch (subclass)
        {
        case 0:
            return "RAM memory";
        case 1:
            return "FLASH memory";
        case 2:
            return "CXL memory controller";
        case 0x80:
            return "Generic memory controller";
        default:
            return "Unknown memory controller";
        }

    case PCI_BRIDGE:
        switch (subclass)
        {
        case 0:
            return "Host bridge";
        case 1:
            return "ISA bridge";
        case 2:
            return "EISA bridge";
        case 3:
            return "MicroChanngel bridge";
        case 4:
            return "PCI bridge";
        case 5:
            return "PCMCIA bridge";
        case 6:
            return "NuBus bridge";
        case 7:
            return "CardBus bridge";
        case 8:
            return "RACEway bridge";
        case 9:
            return "Semi transparent PCI-to-PCI bridge";
        case 0xA:
            return "Infiniband to PCI host bridge";
        case 0x80:
            return "Generic bridge";
        default:
            return "Unknown bridge";
        }

    case PCI_COMMUNICATION_CONTROLLER:
        switch (subclass)
        {
        case 0:
            return "Serial controller";
        case 1:
            return "Parallel controller";
        case 2:
            return "Multiport serial controller";
        case 3:
            return "Modem";
        case 4:
            return "GPIB controller";
        case 5:
            return "Smart Card controller";
        case 0x80:
            return "Generic communication device";
        default:
            return "Unknown communication device";
        }

    case PCI_GENERIC_SYSTEM_PERIPHERAL:
        switch (subclass)
        {
        case 0:
            return "PIC";
        case 1:
            return "DMA controller";
        case 2:
            return "Timer";
        case 3:
            return "RTC";
        case 4:
            return "PCI hot-plug controller";
        case 5:
            return "SD host controller";
        case 6:
            return "IOMMU";
        case 0x80:
            return "Generic system peripheral";
        case 0x90:
            return "Timing card";
        default:
            return "Unknown system peripheral";
        }

    case PCI_INPUT_DEVICE_CONTROLLER:
        switch (subclass)
        {
        case 0:
            return "Keyboard controller";
        case 1:
            return "Digitizer pen";
        case 2:
            return "Mouse controller";
        case 3:
            return "Scanner controller";
        case 4:
            return "Gameport controller";
        case 0x80:
            return "Generic input device controller";
        default:
            return "Unknown input device controller";
        }

    case PCI_DOCKING_STATION:
        switch (subclass)
        {
        case 0:
        case 0x80:
            return "Generic docking station";
        default:
            return "Unknown docking station";
        }

    case PCI_PROCESSOR:
        switch (subclass)
        {
        case 0:
            return "i386";
        case 1:
            return "i486";
        case 2:
            return "Pentium";
        case 0x10:
            return "Alpha";
        case 0x20:
            return "PPC";
        case 0x30:
            return "MIPS";
        case 0x40:
            return "Co-processor";
        default:
            return "Unknown processor";
        }

    case PCI_SERIAL_BUS_CONTROLLER:
        switch (subclass)
        {
        case 0:
            return "FireWire";
        case 1:
            return "ACCESS bus";
        case 2:
            return "SSA";
        case 3:
            return "USB controller";
        case 4:
            return "Fibre channel";
        case 5:
            return "SMBus";
        case 6:
            return "InfiniBand";
        case 7:
            return "IPMI interface";
        case 8:
            return "SERCOS interface";
        case 9:
            return "CANBUS";
        case 0x80:
            return "Generic serial bus controller";
        default:
            return "Unknown serial bus controller";
        }

    case PCI_WIRELESS_CONTROLLER:
        switch (subclass)
        {
        case 0:
            return "IRDA controller";
        case 1:
            return "Consumer IR controller";
        case 0x10:
            return "RF controller";
        case 0x11:
            return "Bluetooth controller";
        case 0x12:
            return "Broadband controller";
        case 0x20:
            return "802.1a controller";
        case 0x21:
            return "802.1b controller";
        case 0x80:
            return "Generic wireless controller";
        default:
            return "Unknown wireless controller";
        }

    case PCI_INTELLIGENT_CONTROLLER:
        switch (subclass)
        {
        case 0:
            return "I2O";
        default:
            return "Unknown intelligent controller";
        }

    case PCI_SATELLITE_COMMUNICATION_CONTROLLER:
        switch (subclass)
        {
        case 1:
            return "Satellite TV controller";
        case 2:
            return "Satellite audio controller";
        case 3:
            return "Satellite voice controller";
        case 4:
            return "Satellite data controller";
        default:
            return "Unknown satellite controller";
        }

    case PCI_ENCRYPTION_CONTROLLER:
        switch (subclass)
        {
        case 0:
            return "Network & computing encryption device";
        case 1:
            return "Entertainment encryption device";
        case 0x80:
            return "Generic encryption controller";
        default:
            return "Unknown encryption controller";
        }

    case PCI_SIGNAL_PROCESSING_CONTROLLER:
        switch (subclass)
        {
        case 0:
            return "DPIO module";
        case 1:
            return "Performance counters";
        case 0x10:
            return "Communication synchronizer";
        case 0x20:
            return "Signal processing management";
        case 0x80:
            return "Generic signal processing controller";
        default:
            return "Unknown signal processing controller";
        }

    case PCI_PROCESSING_ACCELERATOR:
        return "Generic processing accelerator";

    case PCI_NON_ESSENTIAL_INSTRUMENTATION:
        return "Generic non-essential instrumentation";

    case PCI_COPROCESSOR:
        return "Generic coprocessor";

    case PCI_VENDOR_SPECIFIC:
        return "Vendor specific";

    default:
        break;
    }

    return "???";
}

static u32 pci_read_4bytes(u8 bus, u8 dev, u8 func, u8 offset)
{
    if (offset & 0b11)
        return 0;

    PCIDeviceRegister reg = {
        .register_offset = offset,
        .func = func,
        .dev = dev,
        .bus = bus,
        .enabled = 1};

    port_outl(reg.whole, PCI_CONFIG_ADDRESS);
    return port_inl(PCI_CONFIG_DATA);
}

static int pci_register_device(u8 bus, u8 dev, u8 func)
{
    u32 tmp = pci_read_4bytes(bus, dev, func, 0);
    if ((tmp & 0xFFFF) == 0xFFFF)
        return -ENODEV;

    PCIDevice* tmpalloc =
        krealloc(g_pci_devices, (g_pci_devices_count + 1) * sizeof(PCIDevice));
    if (!tmpalloc)
        return -ENOMEM;

    g_pci_devices = tmpalloc;
    ++g_pci_devices_count;

    PCIDevice* device = &g_pci_devices[g_pci_devices_count - 1];
    device->bus = bus;
    device->dev = dev;
    device->func = func;

    device->vendor_id = tmp & 0xFFFF;
    device->device_id = (tmp >> 16) & 0xFFFF;

    tmp = pci_read_4bytes(bus, dev, func, 0x8);
    device->class = (tmp >> 24) & 0xFF;
    device->subclass = (tmp >> 16) & 0xFF;

    KLOGF(
        INFO,
        "(%02X:%02X.%X) %s [%02X:%02X]",
        bus,
        dev,
        func,
        pci_class_to_string(device->class, device->subclass),
        device->class,
        device->subclass);

    return 0;
}

void pci_enumerate_bus(u8 bus)
{
    for (u8 dev = 0; dev < 32; ++dev)
    {
        for (size_t i = 0; i < 8; ++i)
        {
            int st = pci_register_device(bus, dev, i);
            if (st && st != -ENODEV)
            {
                KLOGF(
                    ERR,
                    "Failed to register device (%02X:%02X.%X), error: %d.",
                    bus,
                    dev,
                    i,
                    st);
            }
        }
    }
}

void pci_enumerate_devices()
{
    /* Check root bus to see if it's multi-function. */
    u32 tmp = pci_read_4bytes(0, 0, 0, 0);

    if ((tmp & 0xFFFF) == 0xFFFF)
        panic("Couldn't find root PCI bus."); // how doe?

    pci_enumerate_bus(0);

    tmp = pci_read_4bytes(0, 0, 0, 0xC);
    if ((tmp >> 16) & 0x80)
    {
        for (u8 func = 1; func < 8; ++func)
        {
            if ((pci_read_4bytes(0, 0, func, 0) & 0xFFFF) == 0xFFFF)
                break; /* that's all. */
            pci_enumerate_bus(func);
        }
    }
}
