/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/errno.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/module.h>
#include <dxgmx/panic.h>
#include <dxgmx/pci.h>
#include <dxgmx/utils/linkedlist.h>

#define KLOGF_PREFIX "pci: "

static LinkedList g_pci_devs;
static LinkedList g_pci_dev_drivers;

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

static u16 pci_read_vendor_id(u8 bus, u8 dev, u8 func)
{
    return pci_read_4bytes(bus, dev, func, 0) & 0xFFFF;
}

static u8 pci_read_header_type(u8 bus, u8 dev, u8 func)
{
    return (pci_read_4bytes(bus, dev, func, 0xC) >> 16) & 0xFF;
}

static bool pci_device_is_multifunction(u8 bus, u8 dev)
{
    return pci_read_header_type(bus, dev, 0) & (1 << 7);
}

static bool pci_device_is_host_bridge(u8 bus, u8 dev, u8 func)
{
    u32 tmp = pci_read_4bytes(bus, dev, func, 0x8);
    u8 class = (tmp >> 24) & 0xFF;
    u8 subclass = (tmp >> 16) & 0xFF;
    return class == PCI_BRIDGE && subclass == 0;
}

static int pci_register_device(u8 bus, u8 dev, u8 func)
{
    PCIDevice* device = kmalloc(sizeof(PCIDevice));
    if (!device)
        return -ENOMEM;

    if (linkedlist_add(device, &g_pci_devs) < 0)
    {
        kfree(device);
        return -ENOMEM;
    }

    device->bus = bus;
    device->dev = dev;
    device->func = func;

    u32 tmp = pci_read_4bytes(bus, dev, func, 0);
    device->vendor_id = tmp & 0xFFFF;
    device->device_id = (tmp >> 16) & 0xFFFF;

    tmp = pci_read_4bytes(bus, dev, func, 0x8);
    device->class = (tmp >> 24) & 0xFF;
    device->subclass = (tmp >> 16) & 0xFF;
    device->progif = (tmp >> 8) & 0xFF;
    device->revision_id = tmp & 0xFF;

    tmp = pci_read_4bytes(bus, dev, func, 0xC);
    device->header_type = (tmp >> 16) & 0xFF;

    KLOGF(
        INFO,
        "(%02X:%02X.%X) %s",
        bus,
        dev,
        func,
        pci_class_to_string(device->class, device->subclass));

    return 0;
}

static void pci_enumerate_bus(u8 bus)
{
    for (u8 dev = 0; dev < 32; ++dev)
    {
        if (pci_read_vendor_id(bus, dev, 0) == 0xFFFF)
            continue;

        int st = pci_register_device(bus, dev, 0);
        if (st)
        {
            KLOGF(
                ERR, "Failed to register (%02X:%02X.%X), %d.", bus, dev, 0, st);
            return;
        }

        /* Only keep going if the device has multiple functions. */
        if (!pci_device_is_multifunction(bus, dev))
            continue;

        for (u8 func = 1; func < 8; ++func)
        {
            if (pci_read_vendor_id(bus, dev, func) == 0xFFFF)
                continue;

            st = pci_register_device(bus, dev, func);
            if (st)
            {
                KLOGF(
                    ERR,
                    "Failed to register (%02X:%02X.%X), %d.",
                    bus,
                    dev,
                    func,
                    st);
            }

            if (pci_device_is_host_bridge(bus, dev, func))
                pci_enumerate_bus(func);
        }
    }
}

static int pci_enumerate_devices()
{
    /* Read the vendor ID of the root bus (00:00.0) */
    u16 root_vendor_id = pci_read_vendor_id(0, 0, 0);
    if (root_vendor_id == 0xFFFF)
    {
        KLOGF(WARN, "No PCI root bus!");
        return -ENODEV;
    }

    pci_enumerate_bus(0);

    return 0;
}

int pci_register_device_driver(const PCIDeviceDriver* driver)
{
    if (linkedlist_add((PCIDeviceDriver*)driver, &g_pci_dev_drivers) < 0)
        return -ENOMEM;

    FOR_EACH_ENTRY_IN_LL (g_pci_devs, PCIDevice*, dev)
    {
        if (dev->class == driver->class && dev->subclass == driver->subclass &&
            !dev->driver)
        {
            if (driver->probe(dev) == 0)
            {
                dev->driver = driver;
                break;
            }
        }
    }

    return 0;
}

int pci_unregister_device_driver(const PCIDeviceDriver* driver)
{
    FOR_EACH_ENTRY_IN_LL (g_pci_devs, PCIDevice*, dev)
    {
        if (dev->driver == driver)
            dev->driver = NULL;
    }

    return linkedlist_remove_by_data(
        (PCIDeviceDriver*)driver, &g_pci_dev_drivers);
}

u32 pci_read_bar4(const PCIDevice* dev)
{
    if (dev->header_type != 0)
        return -EINVAL;

    return pci_read_4bytes(dev->bus, dev->dev, dev->func, 0x20);
}

static int pci_main()
{
    return pci_enumerate_devices();
}

static int pci_exit()
{
    return 0;
}

MODULE g_pci_module = {
    .name = "pci", .main = pci_main, .exit = pci_exit, .stage = MODULE_STAGE2};
