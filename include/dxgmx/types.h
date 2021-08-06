
#ifndef _DXGMX_TYPES_H
#define _DXGMX_TYPES_H

#include<stdint.h>

#if defined(_X86_)
typedef uint32_t ptr;
#else if defined(_X86_64_)
typedef uint64_t ptr;
#endif //_X86_

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

#endif //!_DXGMX_TYPES_H
