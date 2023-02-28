/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/pci.h>
#include <dxgmx/x86/portio.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

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

u32 pci_read_4bytes(u8 bus, u8 dev, u8 func, u8 offset)
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
