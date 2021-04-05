
#ifndef __DXGMX_TTY_H__
#define __DXGMX_TTY_H__

#include<stddef.h>

void tty_init();
void tty_clear();
int tty_print(const char *str, size_t n);

#endif // __DXGMX_TTY_H__