/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/errno.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/kstdio/kstdio.h>
#include <dxgmx/kstdio/sink.h>
#include <dxgmx/todo.h>
#include <dxgmx/video/fb.h>

/* Can't dinamically allocate these, since we kmalloc isn't up when most sinks
 * get registered. */
#define SINKS_CAPACITY 4
static KOutputSink* g_sinks[SINKS_CAPACITY];
static size_t g_sink_count;

#define FOR_EACH_SINK_OF_TYPE(_type, _sink)                                    \
    FOR_EACH_ELEM_IN_DARR ((KOutputSink**)g_sinks, g_sink_count, _sink)        \
        if ((*_sink)->type == _type)

static int kstdio_validate_sink(KOutputSink* sink)
{
    /* All sinks should have these set. */
    if (!sink->name || !sink->init || !sink->destroy || !sink->output_char)
        return -EINVAL;

    switch (sink->type)
    {
    case KOUTPUT_TERMINAL:
        if (!sink->newline)
            return -EINVAL;
        break;

    case KOUTPUT_RAW:
        break;

    default:
        return -EINVAL;
    }

    return 0;
}

int kstdio_register_sink(KOutputSink* sink)
{
    if (kstdio_validate_sink(sink) < 0)
        return -EINVAL;

    if (g_sink_count > SINKS_CAPACITY)
        return -ENOMEM;

    int st = sink->init(sink);
    if (st < 0)
        return st;

    g_sinks[g_sink_count++] = sink;

    return 0;
}

int kstdio_unregister_sink(KOutputSink* sink)
{
    int st = sink->destroy(sink);
    if (st < 0)
        return st;

    ssize_t idx = -1;
    for (size_t i = 0; i < g_sink_count; ++i)
    {
        if (g_sinks[i] == sink)
        {
            idx = i;
            break;
        }
    }

    if (idx == -1)
        return -ENOENT;

    for (size_t i = idx; i < g_sink_count - 1; ++i)
        g_sinks[i] = g_sinks[i + 1];

    --g_sink_count;

    return 0;
}

size_t kstdio_write(const char* buf, size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        FOR_EACH_SINK_OF_TYPE (KOUTPUT_RAW, sink)
            (*sink)->output_char(buf[i], *sink);
    }

    for (size_t i = 0; i < n; ++i)
    {
        switch (buf[i])
        {
        case '\n':
            FOR_EACH_SINK_OF_TYPE (KOUTPUT_TERMINAL, sink)
                (*sink)->newline(*sink);
            break;

        case '\t':
            for (size_t k = 0; k < 4; ++k)
            {
                FOR_EACH_SINK_OF_TYPE (KOUTPUT_TERMINAL, sink)
                    (*sink)->output_char(' ', *sink);
            }
            break;

        default:
            FOR_EACH_SINK_OF_TYPE (KOUTPUT_TERMINAL, sink)
                (*sink)->output_char(buf[i], *sink);
            break;
        }
    }

    return n;
}
