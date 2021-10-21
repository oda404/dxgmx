/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/attrs.h>
#include<dxgmx/abandon_ship.h>

/* 
 * This function initiates core hardware and 
 * brings the machine in an operational state.
 * It's implementation is architecture specific 
 * and can be found in arch/<arch>/kinit_stage1.c.
 */
extern int kinit_stage1();
/* This function expects the hardware to be initialized
 * and in working order. It is responsible for initiating
 * kernel specific stuff. It's implementation can be found in 
 * kernel/kinit_stage2.c
 */
extern int kinit_stage2();

_ATTR_NORETURN void kmain()
{
    kinit_stage1();

    kinit_stage2();

    abandon_ship("Tried to return from kmain. Not proceeding.\n");
}
