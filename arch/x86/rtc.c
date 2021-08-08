
#include<dxgmx/x86/rtc.h>
#include<dxgmx/x86/cmos.h>
#include<dxgmx/x86/sysidt.h>
#include<dxgmx/klog.h>
#include<dxgmx/todo.h>
#include<dxgmx/types.h>
#include<dxgmx/string.h>
#include<dxgmx/time.h>

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

static void bcd_to_binary(u8 *val)
{
    *val = ((*val & 0xF0) >> 1) + ((*val & 0xF0) >> 3) + (*val & 0xF);
}

static u16 rtc_calculate_freq()
{
    u8 a = cmos_port_inb(RTC_REG_A, NMIENABLED);
    /* The default freq is 1024Hz which is fine. */
    return 32768 >> ((a & 0b1111) - 1);
}

static RTCTimeInfo g_rtc_timeinfo;
static u16         g_rtc_freq;
static Timespec    g_rtc_timespec;

void rtc_int_handler(
    const InterruptFrame _ATTR_MAYBE_UNUSED *frame, 
    const void _ATTR_MAYBE_UNUSED *data
)
{
    asm volatile("cli");

    u8 b = cmos_port_inb(RTC_REG_B, NMIDISABLED);
    u8 c = cmos_port_inb(RTC_REG_C, NMIDISABLED);

    if(c & RTC_REG_C_UPDATE_DONE)
    {
        g_rtc_timeinfo.seconds  = cmos_port_inb(RTC_REG_SECONDS,  NMIDISABLED);
        g_rtc_timeinfo.minutes  = cmos_port_inb(RTC_REG_MINUTES,  NMIDISABLED);
        g_rtc_timeinfo.hours    = cmos_port_inb(RTC_REG_HOURS,    NMIDISABLED);
        g_rtc_timeinfo.weekday  = cmos_port_inb(RTC_REG_WEEKDAY,  NMIDISABLED);
        g_rtc_timeinfo.monthday = cmos_port_inb(RTC_REG_MONTHDAY, NMIDISABLED);
        g_rtc_timeinfo.month    = cmos_port_inb(RTC_REG_MONTH,    NMIDISABLED);
        g_rtc_timeinfo.year     = cmos_port_inb(RTC_REG_YEAR,     NMIDISABLED);

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

        cmos_enable_nmi();
    }
    else
    {
        timespec_add(1.f / g_rtc_freq * 1000000000, &g_rtc_timespec);
    }

    asm volatile("sti");
}

int rtc_init()
{
    memset(&g_rtc_timeinfo, 0, sizeof(RTCTimeInfo));
    memset(&g_rtc_timespec, 0, sizeof(Timespec));
    g_rtc_freq = rtc_calculate_freq();

    sysidt_register_callback(IRQ8, rtc_int_handler);
    rtc_enable_irq8();
    // read the C register so we make sure future IRQs will fire.
    cmos_port_inb(RTC_REG_C, NMIENABLED);
    while(!g_rtc_timeinfo.year /*2100?*/);

    return 0;
}

void rtc_enable_irq8()
{
    asm volatile("cli");
    
    // to enable irq8 we need to set bit 6 of the B status register.
    u8 val = cmos_port_inb(0xB, NMIENABLED);
    cmos_port_outb(val | (1 << 6), 0xB, NMIENABLED);

    asm volatile("sti");
}

void rtc_dump_timeinfo()
{
    klog(
        KLOG_INFO,
        "[RTC] Current date: %02d:%02d:%02d %02d/%02d/20%d\n",
        g_rtc_timeinfo.hours,
        g_rtc_timeinfo.minutes,
        g_rtc_timeinfo.seconds,
        g_rtc_timeinfo.monthday,
        g_rtc_timeinfo.month,
        g_rtc_timeinfo.year
    );
}

const RTCTimeInfo *rtc_get_timeinfo()
{
    return &g_rtc_timeinfo;
}
