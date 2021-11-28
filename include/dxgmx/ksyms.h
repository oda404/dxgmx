
#ifndef _DXGMX_KSYMS_H
#define _DXGMX_KSYMS_H

#include<dxgmx/types.h>

int ksyms_load();
/** 
 * Writes no more than 'n' characters of the symbol's name that falls under 'addr' in 'name'. 
 * Optinally puts the offset of 'addr' into the symbol base in 'offset'. 
 * @return true if the symbol was found and it's name written, false otherwise. 
*/
size_t ksyms_get_symbol_name(
    ptr addr, 
    ptr *offset, 
    char *name,
    size_t n
);

#endif //!_DXGMX_KSYMS_H
