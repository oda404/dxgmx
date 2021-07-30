/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#ifndef _DXGMX_VIDEO_TTY_H
#define _DXGMX_VIDEO_TTY_H

#include<stddef.h>

void tty_init();
void tty_clear();
size_t tty_print(const char *str, size_t n);

#endif // _DXGMX_VIDEO_TTY_H