/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_X86_RTC_H
#define _DXGMX_X86_RTC_H

#include <dxgmx/time.h>

int rtc_init();
/* Enables periodic interrupts on IRQ line 8. */
void rtc_enable_periodic_int();
/* Disable periodic interrupts. */
void rtc_disable_periodic_int();
bool rtc_periodic_ints_enabled();
/* Returns a timespec struct with an arbitrary timestamp
used for meassuring time passage. If periodic interrupts are disabled,
the RTC can't give a 'now' timestamp, and the returned timespec is
zero'd out. */
struct timespec rtc_now();
/* Retunss a tm struct with the current date. If periodic interrupts are
enabled the tm that was last updated in an IRQ is returned, else the
date is calculated right then. */
struct tm rtc_date();

#endif //_DXGMX_X86_RTC_H
