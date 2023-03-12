/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

/* Initrd PID 1 */

#include <dxgmx.h>
#include <fcntl.h>

int main()
{
    dxgmx_klog("Started initrd PID 1");

    int st = open("/bin/penis", 1, 0);

    return st;
}
