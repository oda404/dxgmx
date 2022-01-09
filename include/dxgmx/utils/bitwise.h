/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_BITWISE_H
#define _DXGMX_BITWISE_H

/* 
 * Different bitwise utilities that are mostly used 
 * in protected mode to operate on 64 bit numbers
 */

#include<dxgmx/types.h>

/* returns 1 if 'n' is aligned on 'align' bytes. */
void bw_clear(u64 *n, u8 bit);
void bw_set(u64 *n, u8 bit);
/* returns > 0 if the given number uses more than 32 bits */
int  bw_is64_wide(u64 n);
u32 bw_u32_rotl(u32 n, u8 rot);
u32 bw_u32_flip_endianness(u32 n);
bool bw_is_power_of_two(u64 n);

#endif // _DXGMX_BITWISE_H
