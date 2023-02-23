/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/timekeep.h>
#include <dxgmx/timer.h>
#include <dxgmx/todo.h>
#include <dxgmx/utils/linkedlist.h>

#define KLOGF_PREFIX "timekeep: "

static Timer g_uptime_timer;
static LinkedList g_timesources;
static TimeSource* g_best_timesource;

int timekeep_register_timesource(TimeSource* ts)
{
    int st = linkedlist_add(ts, &g_timesources);
    if (st < 0)
        return st;

    st = ts->init(ts);
    if (st < 0)
    {
        linkedlist_remove_by_data(ts, &g_timesources);
        return st;
    }

    return 0;
}

int timekeep_unregister_timesource(TimeSource* ts)
{
    ts->destroy(ts);
    return linkedlist_remove_by_data(ts, &g_timesources);
}

_INIT void timekeep_early_init()
{
    /* Here we do some early initilizations for the timekeep. This is needed
     * because TimeSources usually don't come online until much later after
     * calling this function, but timekeep is used regardless by stuff like
     * klog. This way klog will just get 0 until the actual TimeSources kick in.
     */
    timer_start_stub(&g_uptime_timer);
}

_INIT int timekeep_init()
{
    size_t timesource_count = 0;
    FOR_EACH_ENTRY_IN_LL (g_timesources, TimeSource*, ts)
    {
        if (!g_best_timesource || ts->priority > g_best_timesource->priority)
            g_best_timesource = ts;

        ++timesource_count;
    }

    if (!timesource_count)
        panic("timekeep: No (valid) timesources were registered!");

    KLOGF(
        INFO,
        "Using timesource '%s' with priority %d.",
        g_best_timesource->name,
        g_best_timesource->priority);

    /* Start the uptime timer for real. */
    timer_start(&g_uptime_timer);

    return 0;
}

struct timespec timekeep_uptime()
{
    struct timespec ret;
    timer_elapsed(&ret, &g_uptime_timer);
    return ret;
}

TimeSource* timekeep_get_best_timesource()
{
    return g_best_timesource;
}
