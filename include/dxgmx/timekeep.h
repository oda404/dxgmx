/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_TIMEKEEP_H
#define _DXGMX_TIMEKEEP_H

#include <dxgmx/time.h>

typedef struct S_TimeSource
{
    /* Name of this timesource */
    const char* name;

    /* Priority over other timesources. The biggest number gets choosen as the
     * best timesource. */
    u32 priority;

    int (*init)(struct S_TimeSource*);
    int (*destroy)(struct S_TimeSource*);

    struct timespec (*now)();
} TimeSource;

/**
 * Register a timesource, calling it's init.
 *
 * 'ts' Non NULL TimeSource.
 *
 * Returns:
 * 0 on success.
 * -ENOMEM on out memory.
 * Other errors come from the time source's init.
 */
int timekeep_register_timesource(TimeSource* ts);

/**
 * Unregister a timesource, calling it's destroy.
 *
 * 'ts' Non NULL TimeSource.
 *
 * Returns:
 * 0 on success.
 * -ENOENT if the timesource doesn't exist (destroy will still be called).
 */
int timekeep_unregister_timesource(TimeSource* ts);

/**
 * Initialize timekeep after TimeSources were registered. This function will
 * enumerate all TimeSources, find the best one and allow the meaningfull usage
 * of timers.
 *
 * Returns:
 * 0 on success.
 */
int timekeep_init();

/**
 * Get the system uptime. Do not call this function before
 * timekeep_early_init(), it will panic the kernel :). After
 * timekeep_early_init() this will return 0 until timekeep_init() is called.
 */
struct timespec timekeep_uptime();

/**
 * Get the TimeSource the timekeep has deemed to be best. This function will
 * return NULL if called before timekeep_init().
 *
 * Returns:
 * Non NULL TimeSource*
 */
TimeSource* timekeep_get_best_timesource();

#endif //!_DXGMX_TIMEKEEP_H
