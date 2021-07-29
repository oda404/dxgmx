
#include<dxgmx/x86/rtc.h>
#include<dxgmx/x86/cmos.h>
#include<dxgmx/x86/portio.h>
#include<dxgmx/x86/sysidt.h>
#include<dxgmx/klog.h>
#include<dxgmx/assert.h>
#include<dxgmx/todo.h>

#define RTC_REG_A 0xA
#define RTC_REG_B 0xB
#define RTC_REG_C 0xC

#define RTC_REG_SECONDS  0x0
#define RTC_REG_MINUTES  0x2
#define RTC_REG_HOURS    0x4
#define RTC_REG_WEEKDAY  0x6
#define RTC_REG_MONTHDAY 0x7
#define RTC_REG_MONTH    0x8
#define RTC_REG_YEAR     0x9

typedef enum
E_RTCRegisterCFlags
{
    RTC_REG_C_UPDATE_DONE = (1 << 4)
} RTCRegisterCFlags;

typedef enum
E_RTCRegisterBFlags
{
    RTC_REG_B_24HOUR_FMT  = (1 << 1),
    RTC_REG_B_BINARY_MODE = (1 << 2)
} RTCRegisterBFlags;

static void bcd_to_binary(uint8_t *val)
{
    *val = ((*val & 0xF0) >> 1) + ((*val & 0xF0) >> 3) + (*val & 0xF);
}

static RTCTimeInfo g_rtc_timeinfo;

void rtc_int_handler(const InterruptFrame *frame, const void *data)
{
    cmos_disable_nmi();
    asm volatile("cli");

    uint8_t b = cmos_port_inb(RTC_REG_B);
    uint8_t c = cmos_port_inb(RTC_REG_C);

    if(c & RTC_REG_C_UPDATE_DONE)
    {
        g_rtc_timeinfo.isvalid  = 1;
        g_rtc_timeinfo.seconds  = cmos_port_inb(RTC_REG_SECONDS);
        g_rtc_timeinfo.minutes  = cmos_port_inb(RTC_REG_MINUTES);
        g_rtc_timeinfo.hours    = cmos_port_inb(RTC_REG_HOURS);
        g_rtc_timeinfo.weekday  = cmos_port_inb(RTC_REG_WEEKDAY);
        g_rtc_timeinfo.monthday = cmos_port_inb(RTC_REG_MONTHDAY);
        g_rtc_timeinfo.month    = cmos_port_inb(RTC_REG_MONTHDAY);
        g_rtc_timeinfo.year     = cmos_port_inb(RTC_REG_YEAR);

        if(!(b & RTC_REG_B_BINARY_MODE))
        {
            bcd_to_binary(&g_rtc_timeinfo.seconds);
            bcd_to_binary(&g_rtc_timeinfo.minutes);
            bcd_to_binary(&g_rtc_timeinfo.hours);
            bcd_to_binary(&g_rtc_timeinfo.weekday);
            bcd_to_binary(&g_rtc_timeinfo.monthday);
            bcd_to_binary(&g_rtc_timeinfo.month);
            bcd_to_binary(&g_rtc_timeinfo.year);
        }

        if(!(b & RTC_REG_B_24HOUR_FMT))
        {
            if(g_rtc_timeinfo.hours & (1 << 7))
            {
                //PM
                TODO();
            }
        }
    }

    asm volatile("sti");
    cmos_enable_nmi();
}

int rtc_init()
{
    g_rtc_timeinfo.isvalid = 0;

    sysidt_register_callback(IRQ8, rtc_int_handler);
    rtc_enable_irq8();
    // read the C register so we make sure future IRQs will fire.
    cmos_port_inb(RTC_REG_C);

    rtc_dump_time_info();

    return 0;
}

void rtc_enable_irq8()
{
    asm volatile("cli");
    
    // to enable irq8 we need to set bit 6 of the B status register.
    uint8_t val = cmos_port_inb(0x8B);
    port_outb(0x8B, CMOS_PORT_REG);
    port_outb(val | (1 << 6), CMOS_PORT_DATA);

    cmos_enable_nmi();

    asm volatile("sti");
}

void rtc_dump_time_info()
{
    while(!g_rtc_timeinfo.isvalid);
    klog(
        KLOG_INFO,
        "[RTC] Current date: %d:%d:%d %d/%d/%d\n",
        g_rtc_timeinfo.hours,
        g_rtc_timeinfo.minutes,
        g_rtc_timeinfo.seconds,
        g_rtc_timeinfo.monthday,
        g_rtc_timeinfo.month,
        g_rtc_timeinfo.year
    );
}
