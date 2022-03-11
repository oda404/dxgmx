/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_TIMEKEEP_H
#define _DXGMX_TIMEKEEP_H

#include <dxgmx/time.h>

int timekeep_init();
/* Returns the current date. */
struct tm timkeep_date();
/* Returns the uptime since timekeep_init() was called. */
struct timespec timekeep_uptime();
/* Returns the current unixts in a struct timespec. */
struct timespec timekeep_unixts();

#endif //!_DXGMX_TIMEKEEP_H
