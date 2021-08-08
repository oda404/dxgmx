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

#define RTC_REG_SECONDS  0x0 /* seconds [0-59]        */
#define RTC_REG_MINUTES  0x2 /* minutes [0-59]        */
#define RTC_REG_HOURS    0x4 /* hours   [0-23]/[1-12] */
#define RTC_REG_WEEKDAY  0x6 /* wday    [1-7]         */
#define RTC_REG_MONTHDAY 0x7 /* mday    [1-31]        */
#define RTC_REG_MONTH    0x8 /* month   [1-12]        */
#define RTC_REG_YEAR     0x9 /* year    [0-99]        */

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

static void bcd_to_binary(int *val)
{
    *val = ((*val & 0xF0) >> 1) + ((*val & 0xF0) >> 3) + (*val & 0xF);
}

static u16 rtc_calculate_freq()
{
    u8 a = cmos_port_inb(RTC_REG_A, NMIENABLED);
    /* The default freq is 1024Hz which is fine. */
    return 32768 >> ((a & 0b1111) - 1);
}

static struct tm   g_rtc_tm;
static u16         g_rtc_freq;
static Timespec    g_rtc_ts;

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
        g_rtc_tm.tm_sec  = cmos_port_inb(RTC_REG_SECONDS,  NMIDISABLED);
        g_rtc_tm.tm_min  = cmos_port_inb(RTC_REG_MINUTES,  NMIDISABLED);
        g_rtc_tm.tm_hour = cmos_port_inb(RTC_REG_HOURS,    NMIDISABLED);
        g_rtc_tm.tm_wday = cmos_port_inb(RTC_REG_WEEKDAY,  NMIDISABLED) - 1;
        g_rtc_tm.tm_mday = cmos_port_inb(RTC_REG_MONTHDAY, NMIDISABLED);
        g_rtc_tm.tm_mon  = cmos_port_inb(RTC_REG_MONTH,    NMIDISABLED) - 1;
        g_rtc_tm.tm_year = cmos_port_inb(RTC_REG_YEAR,     NMIDISABLED);
        g_rtc_tm.tm_yday = 0; //FIXME
        g_rtc_tm.tm_isdst = 0; //FIXME

        if(!(b & RTC_REG_B_BINARY_MODE))
        {
            bcd_to_binary(&g_rtc_tm.tm_sec);
            bcd_to_binary(&g_rtc_tm.tm_min);
            bcd_to_binary(&g_rtc_tm.tm_hour);
            bcd_to_binary(&g_rtc_tm.tm_wday);
            bcd_to_binary(&g_rtc_tm.tm_mday);
            bcd_to_binary(&g_rtc_tm.tm_mon);
            bcd_to_binary(&g_rtc_tm.tm_year);
        }

        /** 
         * Make g_rtc_tm.tm_year be the years since 1900 as per spec. 
         * Just assume we are in the 2000's. 2000 - 1900 == 100.
        */
        g_rtc_tm.tm_year += 100;

        if(!(b & RTC_REG_B_24HOUR_FMT))
        {
            if(g_rtc_tm.tm_hour & (1 << 7))
            {
                //PM
                TODO();
            }
        }

        cmos_enable_nmi();
    }
    else
    {
        timespec_add(1.f / g_rtc_freq * 1000000000, &g_rtc_ts);
    }

    asm volatile("sti");
}

int rtc_init()
{
    memset(&g_rtc_tm, 0, sizeof(struct tm));
    memset(&g_rtc_ts, 0, sizeof(Timespec));
    g_rtc_freq = rtc_calculate_freq();

    sysidt_register_callback(IRQ8, rtc_int_handler);
    rtc_enable_irq8();
    // read the C register so we make sure future IRQs will fire.
    cmos_port_inb(RTC_REG_C, NMIENABLED);
    while(!g_rtc_tm.tm_year /*2100?*/);

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

void rtc_dump_time_and_date()
{
    klog(
        KLOG_INFO,
        "[RTC] Current time and date: %02d:%02d:%02d %02d/%02d/%d.\n",
        g_rtc_tm.tm_hour,
        g_rtc_tm.tm_min,
        g_rtc_tm.tm_sec,
        g_rtc_tm.tm_mday,
        g_rtc_tm.tm_mon + 1,
        1900 + g_rtc_tm.tm_year
    );
}

const Timespec *rtc_get_running_ts()
{
    return &g_rtc_ts;
}

const struct tm *rtc_get_tm()
{
    return &g_rtc_tm;
}