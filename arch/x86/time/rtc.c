/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/x86/rtc.h>
#include<dxgmx/x86/cmos.h>
#include<dxgmx/x86/idt.h>
#include<dxgmx/x86/interrupts.h>
#include<dxgmx/todo.h>
#include<dxgmx/types.h>
#include<dxgmx/string.h>
#include<dxgmx/time.h>
#include<dxgmx/klog.h>
#include<dxgmx/attrs.h>

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

#define RTC_REG_C_DATE_UPDATE_DONE  (1 << 4)
#define RTC_REG_C_PERIODIC_INT_DONE (1 << 6)

#define RTC_REG_B_24HOUR_FMT   (1 << 1)
#define RTC_REG_B_BINARY_MODE  (1 << 2)
#define RTC_REG_B_PERIODIC_INT_ENABLED (1 << 6)

#define KLOGF(lvl, fmt, ...) klogln(lvl, "rtc: " fmt, ##__VA_ARGS__)

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

static u16 rtc_set_freq(RTCFreq freq)
{
    /**
     * Most RTCs give trash results if they're set to fire irq's at
     * more than 8KHz so we cap them at that. 
    */
    if(freq == RTC_FREQ_16KHZ || freq == RTC_FREQ_32KHZ)
        freq = RTC_FREQ_8KHZ;

    u8 a = cmos_port_inb(RTC_REG_A, NMIDISABLED);
    cmos_port_outb((a & 0xF0) | (freq & 0xF) , RTC_REG_A, NMIENABLED);

    return 32768 >> (freq - 1);
}

static u16                      g_freq = 8192;
static volatile struct tm       g_periodic_int_date;
static volatile struct timespec g_periodic_int_timespec;
static bool                     g_periodic_int_enabled = false;

static void rtc_isr(
    const InterruptFrame _ATTR_MAYBE_UNUSED *frame, 
    const void _ATTR_MAYBE_UNUSED *data
)
{
    u8 c = cmos_port_inb(RTC_REG_C, NMIENABLED);

    if(c & RTC_REG_C_PERIODIC_INT_DONE)
    {
        g_periodic_int_timespec.tv_nsec += 1.f / g_freq * 1000000000;
        while(g_periodic_int_timespec.tv_nsec >= 1000000000)
        {
            ++g_periodic_int_timespec.tv_sec;
            g_periodic_int_timespec.tv_nsec -= 1000000000;
        }
    }

    if(c & RTC_REG_C_DATE_UPDATE_DONE)
    {
        /* Hang interrupts so we make sure the time is read correctly. */
        interrupts_disable();

        g_periodic_int_date.tm_sec  = cmos_port_inb(RTC_REG_SECONDS,  NMIDISABLED);
        g_periodic_int_date.tm_min  = cmos_port_inb(RTC_REG_MINUTES,  NMIDISABLED);
        g_periodic_int_date.tm_hour = cmos_port_inb(RTC_REG_HOURS,    NMIDISABLED);
        g_periodic_int_date.tm_wday = cmos_port_inb(RTC_REG_WEEKDAY,  NMIDISABLED);
        g_periodic_int_date.tm_mday = cmos_port_inb(RTC_REG_MONTHDAY, NMIDISABLED);
        g_periodic_int_date.tm_mon  = cmos_port_inb(RTC_REG_MONTH,    NMIDISABLED);
        g_periodic_int_date.tm_year = cmos_port_inb(RTC_REG_YEAR,     NMIDISABLED);
        g_periodic_int_date.tm_yday = 0; //FIXME
        g_periodic_int_date.tm_isdst = 0; //FIXME

        u8 b = cmos_port_inb(RTC_REG_B, NMIDISABLED);

        if(!(b & RTC_REG_B_BINARY_MODE))
        {
            /* It's ok to cast them to int* becuase we know they won't change. */
            bcd_to_binary((int*)&g_periodic_int_date.tm_sec);
            bcd_to_binary((int*)&g_periodic_int_date.tm_min);
            bcd_to_binary((int*)&g_periodic_int_date.tm_hour);
            bcd_to_binary((int*)&g_periodic_int_date.tm_wday);
            --g_periodic_int_date.tm_wday;
            bcd_to_binary((int*)&g_periodic_int_date.tm_mday);
            bcd_to_binary((int*)&g_periodic_int_date.tm_mon);
            --g_periodic_int_date.tm_mon;
            bcd_to_binary((int*)&g_periodic_int_date.tm_year);
        }

        /** 
         * Make g_periodic_int_date.tm_year be the years since 1900 as per spec. 
         * Just assume we are in the 2000's. 2000 - 1900 == 100.
        */
        g_periodic_int_date.tm_year += 100;

        if(!(b & RTC_REG_B_24HOUR_FMT))
        {
            if(g_periodic_int_date.tm_hour & (1 << 7))
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

_INIT int rtc_init()
{
    g_freq = rtc_set_freq(RTC_FREQ_8KHZ);

    return 0;
}

void rtc_enable_periodic_int()
{
    g_periodic_int_enabled = true;

    memset((void*)&g_periodic_int_date, 0, sizeof(struct tm));
    memset((void*)&g_periodic_int_timespec, 0, sizeof(struct timespec));

    idt_register_isr(IRQ8, rtc_isr);
    u8 val = cmos_port_inb(RTC_REG_B, NMIDISABLED);
    cmos_port_outb(val | RTC_REG_B_PERIODIC_INT_ENABLED, RTC_REG_B, NMIENABLED);
    // read the C register so we make sure future IRQs will fire.
    cmos_port_inb(RTC_REG_C, NMIENABLED);

    /* Wait for the first date to be updated. */
    while(!g_periodic_int_date.tm_sec);
}

void rtc_disable_periodic_int()
{
    g_periodic_int_enabled = false;
    u8 val = cmos_port_inb(RTC_REG_B, NMIDISABLED);
    cmos_port_outb(val & ~RTC_REG_B_PERIODIC_INT_ENABLED, RTC_REG_B, NMIENABLED);
}

struct timespec rtc_now()
{
    return g_periodic_int_enabled ? 
        g_periodic_int_timespec :
        (struct timespec) { .tv_sec = 0, .tv_nsec = 0 };
}

struct tm rtc_date()
{
    if(g_periodic_int_enabled)
        return g_periodic_int_date;
    else
        TODO_FATAL();
}

void rtc_dump_date()
{
    const struct tm date = rtc_date();
    KLOGF(
        INFO,
        "Current date is %02d:%02d:%02d %02d/%02d/%d.",
        date.tm_hour,
        date.tm_min,
        date.tm_sec,
        date.tm_mday,
        date.tm_mon + 1,
        1900 + date.tm_year
    );
}

bool rtc_periodic_ints_enabled()
{
    return g_periodic_int_enabled;
}
