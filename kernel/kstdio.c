/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

/* FIXME: this file should be architecture agnostic, but it includes
 * architecture specifc stuff. We should separate this stuff into drivers. */

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

int kstdio_register_sink(KOutputSink* sink)
{
    if (g_sink_count > SINKS_CAPACITY)
        return -ENOMEM;

    g_sinks[g_sink_count++] = sink;

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
