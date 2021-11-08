
#ifndef _DXGMX_ASM_H
#define _DXGMX_ASM_H

#if defined(asm)
#   define ASM asm
#elif defined(__asm)
#   define ASM __asm
#else
#   define ASM __asm__
#endif // defined(asm)

#endif //!_DXGMX_ASM_H
