/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_X86_SERIAL_H
#define _DXGMX_X86_SERIAL_H

#include <dxgmx/types.h>

typedef enum E_SerialPortBaudRate
{
    SERIAL_BAUD_115200 = 1,
    SERIAL_BAUD_57600 = 2,
    SERIAL_BAUD_38400 = 3,
    SERIAL_BAUD_19200 = 6,
    SERIAL_BAUD_9600 = 12,
    SERIAL_BAUD_2400 = 48
} SerialPortBaudRate;

typedef enum E_SerialPortDataBits
{
    SERIAL_DATA_BITS_5 = 0,
    SERIAL_DATA_BITS_6 = 0b01,
    SERIAL_DATA_BITS_7 = 0b10,
    SERIAL_DATA_BITS_8 = 0b11
} SerialPortDataBits;

typedef enum E_SerialPortStopBits
{
    SERIAL_STOP_BITS_1 = 0,
    SERIAL_STOP_BITS_2 = 0b100
} SerialPortStopBits;

typedef enum E_SerialPortParity
{
    SERIAL_PARITY_NONE = 0,
    SERIAL_PARITY_ODD = 0b001000,
    SERIAL_PARITY_EVEN = 0b011000,
    SERIAL_PARITY_MARK = 0b101000,
    SERIAL_PARITY_SPACE = 0b111000
} SerialPortParity;

typedef struct S_SerialPort
{
    u16 port;
    SerialPortParity parity;
    SerialPortBaudRate baudrate;
    SerialPortDataBits databits;
    SerialPortStopBits stopbits;
} SerialPort;

bool serial_config_port(const SerialPort* config);
void serial_write(u8 byte, u16 port);

#endif // !_DXGMX_X86_SERIAL_H
