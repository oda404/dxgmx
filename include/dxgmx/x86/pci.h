/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_X86_PCI_H
#define _DXGMX_X86_PCI_H

#include<dxgmx/types.h>
#include<dxgmx/compiler_attrs.h>

typedef enum 
E_PCIClassCode
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

typedef union
U_PCIDevice
{
    u32 whole;
    _ATTR_PACKED struct
    {
        u8 register_offset;
        u8 func_number: 3;
        u8 dev_number: 5;
        u8 bus_number;
        u8 reserved: 7;
        u8 enabled: 1;
    };
} PCIDevice;

typedef struct
S_PCIDeviceInfo
{
    u8 bus_num;
    u8 device_num;
    u8 func_num;

    u16 vendor_id;
    u16 device_id;
    u8 class;
    u8 subclass;

    const char *name;
} PCIDeviceInfo;

void pci_enumerate_bus(u8 bus);
void pci_enumerate_devices();

#endif // !_DXGMX_X86_PCI_H
