
#ifndef _DXGMX_X86_RTC_H
#define _DXGMX_X86_RTC_H

#include<stdint.h>

typedef struct
S_RTCTimeInfo
{
    uint8_t isvalid;
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t weekday;
    uint8_t monthday;
    uint8_t month;
    uint8_t year;
} RTCTimeInfo;

int rtc_init();
void rtc_enable_irq8();
void rtc_signal_eoi();
void rtc_dump_time_info();
const RTCTimeInfo *rtc_get_time_info();

#endif //_DXGMX_X86_RTC_H