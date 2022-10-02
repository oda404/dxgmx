/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_X86_PCI_H
#define _DXGMX_X86_PCI_H

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
    PCI_SIMPLE_COMMUNICATION_CONTROLLER,
    PCI_BASE_SYSTEM_PERIPHERAL,
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

    PCI_CO_PROCESSOR = 0x40,

    PCI_VENDOR_SPECIFIC = 0xFF
} PCIClassCode;

typedef struct S_PCIDevice
{
    u8 bus;
    u8 dev;
    u8 func;

    u16 vendor_id;
    u16 device_id;
    u8 class;
    u8 subclass;
} PCIDevice;

void pci_enumerate_bus(u8 bus);
void pci_enumerate_devices();

#endif // !_DXGMX_X86_PCI_H
