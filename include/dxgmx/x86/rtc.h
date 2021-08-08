
#ifndef _DXGMX_X86_RTC_H
#define _DXGMX_X86_RTC_H

#include<stdint.h>
#include<dxgmx/time.h>

int rtc_init();
void rtc_enable_irq8();
void rtc_dump_time_and_date();
/**
 * Returns a ptr to the running Timespec struct which is 
 * updated every 1 / rtc_freq seconds.
*/
const Timespec *rtc_get_running_ts();
const struct tm *rtc_get_tm();

#endif //_DXGMX_X86_RTC_H
