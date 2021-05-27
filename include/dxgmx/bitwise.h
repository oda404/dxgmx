
#ifndef __DXGMX_BITWISE_H__
#define __DXGMX_BITWISE_H__

/* 
 * Different bitwise utilities that are mostly used 
 * in protected mode to operate on 64 bit numbers
 */

#include<stdint.h>

void bw_clear(uint64_t *n, uint8_t bit);
void bw_set(uint64_t *n, uint8_t bit);
/* returns > 0 if the given number uses more than 32 bits */
int  bw_is64_wide(uint64_t n);

#endif // __DXGMX_BITWISE_H__
