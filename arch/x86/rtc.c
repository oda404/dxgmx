/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/x86/rtc.h>
#include<dxgmx/x86/cmos.h>
#include<dxgmx/x86/sysidt.h>
#include<dxgmx/x86/interrupts.h>
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
    RTC_REG_C_UPDATE_DONE  = (1 << 4),
    RTC_REG_C_PERIODIC_INT = (1 << 6)
} RTCRegisterCFlags;

typedef enum
E_RTCRegisterBFlags
{
    RTC_REG_B_24HOUR_FMT  = (1 << 1),
    RTC_REG_B_BINARY_MODE = (1 << 2)
} RTCRegisterBFlags;

typedef enum
E_RTCFreq
{
    RTC_FREQ_0HZ,
    RTC_FREQ_32KHZ,
    RTC_FREQ_16KHZ,
    RTC_FREQ_8KHZ,
    RTC_FREQ_4KHZ, 
    RTC_FREQ_2KHZ,
    RTC_FREQ_1KHZ,
    RTC_FREQ_512HZ,
    RTC_FREQ_256HZ,
    RTC_FREQ_128HZ,
    RTC_FREQ_64HZ,
    RTC_FREQ_32HZ,
    RTC_FREQ_16HZ,
    RTC_FREQ_8HZ,
    RTC_FREQ_4HZ,
    RTC_FREQ_2HZ
} RTCFreq;

static void bcd_to_binary(int *val)
{
    *val = ((*val & 0xF0) >> 1) + ((*val & 0xF0) >> 3) + (*val & 0xF);
}

static u16 rtc_calculate_running_freq()
{
    u8 a = cmos_port_inb(RTC_REG_A, NMIENABLED);
    return 32768 >> ((a & 0b1111) - 1);
}

static u16 rtc_set_freq(u8 freq)
{
    /**
     * The RTC gives trash results if it's set to fire irq's at
     * 16KHZ or 32KHZ so we cap it at 8KHz. 
    */
    if(freq == RTC_FREQ_16KHZ || freq == RTC_FREQ_32KHZ)
        freq = RTC_FREQ_8KHZ;

    u8 a = cmos_port_inb(RTC_REG_A, NMIDISABLED);
    a = freq & 1 ? a | 1 : a & ~1;
    a = freq & 2 ? a | 2 : a & ~2;
    a = freq & 4 ? a | 4 : a & ~4;
    a = freq & 8 ? a | 8 : a & ~8;
    cmos_port_outb(a, RTC_REG_A, NMIENABLED);

    return freq;
}

static volatile struct tm       g_rtc_tm;
static u16                      g_rtc_running_freq;
static u16                      g_rtc_base_freq;
static volatile struct timespec g_rtc_ts;

static void rtc_int_handler(
    const InterruptFrame _ATTR_MAYBE_UNUSED *frame, 
    const void _ATTR_MAYBE_UNUSED *data
)
{
    u8 c = cmos_port_inb(RTC_REG_C, NMIENABLED);

    if(c & RTC_REG_C_PERIODIC_INT)
    {
        g_rtc_ts.tv_nsec += 1.f / g_rtc_running_freq * 1000000000;
        while(g_rtc_ts.tv_nsec >= 1000000000)
        {
            ++g_rtc_ts.tv_sec;
            g_rtc_ts.tv_nsec -= 1000000000;
        }
    }

    if(c & RTC_REG_C_UPDATE_DONE)
    {
        /* Hang interrupts so we make sure the time is read correctly. */
        interrupts_disable();

        g_rtc_tm.tm_sec  = cmos_port_inb(RTC_REG_SECONDS,  NMIDISABLED);
        g_rtc_tm.tm_min  = cmos_port_inb(RTC_REG_MINUTES,  NMIDISABLED);
        g_rtc_tm.tm_hour = cmos_port_inb(RTC_REG_HOURS,    NMIDISABLED);
        g_rtc_tm.tm_wday = cmos_port_inb(RTC_REG_WEEKDAY,  NMIDISABLED) - 1;
        g_rtc_tm.tm_mday = cmos_port_inb(RTC_REG_MONTHDAY, NMIDISABLED);
        g_rtc_tm.tm_mon  = cmos_port_inb(RTC_REG_MONTH,    NMIDISABLED) - 1;
        g_rtc_tm.tm_year = cmos_port_inb(RTC_REG_YEAR,     NMIDISABLED);
        g_rtc_tm.tm_yday = 0; //FIXME
        g_rtc_tm.tm_isdst = 0; //FIXME

        u8 b = cmos_port_inb(RTC_REG_B, NMIDISABLED);

        if(!(b & RTC_REG_B_BINARY_MODE))
        {
            /* It's ok to cast them to int* becuase we know they won't change. */
            bcd_to_binary((int*)&g_rtc_tm.tm_sec);
            bcd_to_binary((int*)&g_rtc_tm.tm_min);
            bcd_to_binary((int*)&g_rtc_tm.tm_hour);
            bcd_to_binary((int*)&g_rtc_tm.tm_wday);
            bcd_to_binary((int*)&g_rtc_tm.tm_mday);
            bcd_to_binary((int*)&g_rtc_tm.tm_mon);
            bcd_to_binary((int*)&g_rtc_tm.tm_year);
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

        /* The time has been read and set, interrupts can be turned back on. */
        cmos_enable_nmi();
        interrupts_enable();
    }
}

int rtc_init()
{
    memset((void*)&g_rtc_tm, 0, sizeof(struct tm));
    memset((void*)&g_rtc_ts, 0, sizeof(struct timespec));

    rtc_set_freq(RTC_FREQ_8KHZ);
    g_rtc_running_freq = rtc_calculate_running_freq();
    g_rtc_base_freq = 32768;

    sysidt_register_callback(IRQ8, rtc_int_handler);
    rtc_enable_irq8();
    // read the C register so we make sure future IRQs will fire.
    cmos_port_inb(RTC_REG_C, NMIENABLED);
    while(!g_rtc_tm.tm_year /*2100?*/);

    return 0;
}

void rtc_enable_irq8()
{
    // to enable irq8 we need to set bit 6 of the B status register.
    u8 val = cmos_port_inb(0xB, NMIDISABLED);
    cmos_port_outb(val | (1 << 6), 0xB, NMIENABLED);
}

void rtc_dump_time_and_date()
{
    interrupts_disable();
    cmos_disable_nmi();
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
    cmos_enable_nmi();
    interrupts_enable();
}

const volatile struct timespec *rtc_get_running_ts()
{
    return &g_rtc_ts;
}

const volatile struct tm *rtc_get_tm()
{
    return &g_rtc_tm;
}

u16 rtc_get_running_freq()
{
    return g_rtc_running_freq;
}

u16 rtc_get_base_freq()
{
    return g_rtc_base_freq;
}
