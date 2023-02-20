
#ifndef _DXGMX_KSTDIO_SINK_H
#define _DXGMX_KSTDIO_SINK_H

#include <dxgmx/types.h>

typedef enum E_KOutputType
{
    KOUTPUT_TERMINAL = 1,
    KOUTPUT_RAW = 2
} KOutputType;

typedef enum E_KOutputColor
{
    KOUTPUT_BLACK,
    KOUTPUT_RED,
    KOUTPUT_GREEN,
    KOUTPUT_YELLOW,
    KOUTPUT_BLUE,
    KOUTPUT_MAGENTA,
    KOUTPUT_CYAN,
    KOUTPUT_WHITE
} KOutputColor;

typedef struct S_KOutputSink
{
    /* Name of this sink */
    char* name;

    /* Sink type. Can either be KOUTPUT_TERMINAL, in which case the sink expects
     * the output to first go through a terminal emulator that parses ANSI
     * escape codes, or KOUTPUT_RAW, in which the sink expects all raw bytes.
     * Note that KOUTPUT_RAW sinks are always going to run before
     * KOUTPUT_TERMINAL sinks.
     */
    u8 type;

    /**
     * Initialize the sink.
     *
     * 'sink' Non NULL sink to be initialized.
     *
     * Should return:
     * 0 on success.
     */
    int (*init)(struct S_KOutputSink* sink);

    /**
     * Implementation for outputing a character.
     *
     * 'c' Character to output.
     * 'sink' Non NULL target sink.
     *
     * Should return:
     * 0 on success.
     */
    int (*output_char)(char c, struct S_KOutputSink* sink);

    /* KOUTPUT_TERMINAL specifc stuff */

    /* Foreground color of characters */
    KOutputColor fgcolor;
    /* Background color of characters */
    KOutputColor bgcolor;

    /**
     * Implementation for displaying a newline, possibly scrolling the text.
     *
     * 'sink' Non NULL target sink.
     *
     * Should return:
     * 0 on success.
     */
    int (*newline)(struct S_KOutputSink* sink);

    /* Free for the driver to store anything it needs. */
    void* ctx;
} KOutputSink;

#endif // !_DXGMX_KSTDIO_SINK_H
