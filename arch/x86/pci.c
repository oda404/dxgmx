/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/panic.h>
#include <dxgmx/x86/pci.h>
#include <dxgmx/x86/portio.h>

#define KLOGF(lvl, fmt, ...) klogln(lvl, "pci: " fmt __VA_OPT__(, ) __VA_ARGS__)

#define ACPI_CONFIG_ADDRESS_PORT 0xCF8
#define ACPI_CONFIG_DATA_PORT 0xCFC

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
        case 1:
            return "IDE Controller";
        case 6:
            return "SATA Controller";
        default:
            break;
        }
        break;

    case PCI_NETWORK_CONTROLLER:
        switch (subclass)
        {
        case 0:
            return "Ethernet Controller";
        default:
            break;
        }
        break;

    case PCI_DISPLAY_CONTROLLER:
        switch (subclass)
        {
        case 0:
            return "VGA Compatible Controller";
        default:
            break;
        }
        break;

    case PCI_BRIDGE:
        switch (subclass)
        {
        case 0:
            return "Host Bridge";
        case 1:
            return "ISA Bridge";
        case 0x80:
            return "Unknown Bridge";
        default:
            break;
        }
        break;

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

    port_outl(reg.whole, ACPI_CONFIG_ADDRESS_PORT);
    return port_inl(ACPI_CONFIG_DATA_PORT);
}

static void pci_register_device(u8 bus, u8 dev, u8 func)
{
    u32 tmp = pci_read_4bytes(bus, dev, func, 0);
    if ((tmp & 0xFFFF) == 0xFFFF)
        return;

    g_pci_devices =
        krealloc(g_pci_devices, (++g_pci_devices_count) * sizeof(PCIDevice));
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
}

void pci_enumerate_bus(u8 bus)
{
    for (u8 dev = 0; dev < 32; ++dev)
    {
        for (size_t i = 0; i < 8; ++i)
            pci_register_device(bus, dev, i);
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
