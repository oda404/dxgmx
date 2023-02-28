/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_PCI_PCI_H
#define _DXGMX_PCI_PCI_H

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/types.h>

typedef enum E_PCIClassCode
{
    PCI_UNCLASSIFIED,
    PCI_MASS_STORAGE_CONTROLLER,
    PCI_NETWORK_CONTROLLER,
    PCI_DISPLAY_CONTROLLER,
    PCI_MULTIMEDIA_CONTROLLER,
    PCI_MEMORY_CONTROLLER,
    PCI_BRIDGE,
    PCI_COMMUNICATION_CONTROLLER,
    PCI_GENERIC_SYSTEM_PERIPHERAL,
    PCI_INPUT_DEVICE_CONTROLLER,
    PCI_DOCKING_STATION,
    PCI_PROCESSOR,
    PCI_SERIAL_BUS_CONTROLLER,
    PCI_WIRELESS_CONTROLLER,
    PCI_INTELLIGENT_CONTROLLER,
    PCI_SATELLITE_COMMUNICATION_CONTROLLER,
    PCI_ENCRYPTION_CONTROLLER,
    PCI_SIGNAL_PROCESSING_CONTROLLER,
    PCI_PROCESSING_ACCELERATOR,
    PCI_NON_ESSENTIAL_INSTRUMENTATION,

    PCI_COPROCESSOR = 0x40,

    PCI_VENDOR_SPECIFIC = 0xFF
} PCIClassCode;

struct S_PCIDeviceDriver;

typedef struct S_PCIDevice
{
    /* The bus number where this device resides */
    u8 bus;
    /* The device number on the bus */
    u8 dev;
    /* The function. */
    u8 func;

    u16 vendor_id;
    u16 device_id;

    u8 class;
    u8 subclass;
    u8 revision_id;
    u8 progif;

    u8 header_type;

    const struct S_PCIDeviceDriver* driver;
} PCIDevice;

typedef struct S_PCIDeviceDriver
{
    const char* name;

    u8 class;
    u8 subclass;

    int (*probe)(PCIDevice* dev);
} PCIDeviceDriver;

/* Platform specific */
u32 pci_read_4bytes(u8 bus, u8 dev, u8 func, u8 offset);

int pci_register_device_driver(const PCIDeviceDriver* driver);

u32 pci_read_bar4(const PCIDevice* dev);

#endif // !_DXGMX_PCI_PCI_H
