
#include <dxgmx/klog.h>
#include <dxgmx/kstdio/kstdio.h>
#include <dxgmx/kstdio/sink.h>
#include <dxgmx/module.h>
#include <dxgmx/x86/serial.h>

static int serialsink_init(KOutputSink*)
{
    SerialPort serial_port = {
        .port = 0x3F8,
        .baudrate = SERIAL_BAUD_9600,
        .databits = SERIAL_DATA_BITS_8,
        .stopbits = SERIAL_STOP_BITS_1,
        .parity = SERIAL_PARITY_NONE};

    if (!serial_config_port(&serial_port))
        return -1;

    return 0;
}

static int serialsink_destroy(KOutputSink*)
{
    return 0;
}

static int serialsink_output_char(char c, KOutputSink*)
{
    serial_write(c, 0x3F8);
    return 0;
}

static KOutputSink g_serialsink = {
    .name = "x86-serial",
    .type = KOUTPUT_RAW,
    .init = serialsink_init,
    .destroy = serialsink_destroy,
    .output_char = serialsink_output_char};

static int serialsink_main()
{
    return kstdio_register_sink(&g_serialsink);
}

static int serialsink_exit()
{
    return kstdio_unregister_sink(&g_serialsink);
}

MODULE g_serialsink_module = {
    .name = "x86-serialsink",
    .main = serialsink_main,
    .exit = serialsink_exit,
    .stage = MODULE_STAGE1};
