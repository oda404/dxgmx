/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#ifndef _DXGMX_ATTRS_H
#define _DXGMX_ATTRS_H

#ifdef __GNUC__

/* attr may not be used */
#define _ATTR_MAYBE_UNUSED \
__attribute__((unused))
/* function is pure (does not modify program state nor any passed ptrs) */
#define _ATTR_PURE \
__attribute__((pure))
/*
 * function's return value will not be affected by the program's state, 
 * passing in the same args will always result the same return value
*/
#define _ATTR_CONST \
__attribute__((const))
/* 
 * ptrs passed to the function will be checked by the compiler 
 * to be non-null, if null a warning will be issued
*/
#define _ATTR_NONNULL(...) \
__attribute__((nonnull(__VA_ARGS__)))
/* used on a malloc like function
 * ptr returned by the function cannot alias any other valid pointer
 * opt arg1: dealloc is a suitable deallocation function for ptrs obtained through said malloc-like function
*/
#define _ATTR_MALLOC(...) \
__attribute__((malloc, __VA_ARGS__))
/* used on a malloc-like function
 * arg1: int which represents the size of the memory block
 * pointed to by the returned ptr
*/
#define _ATTR_ALLOC_SIZE(...) \
__attribute__((alloc_size(__VA_ARGS__)))
/*
 * funtion will not return.
*/
#define _ATTR_NORETURN \
__attribute__((noreturn))
/*
 * function will always be inlined even if optimizations are off
*/
#define _ATTR_ALWAYS_INLINE \
__attribute__((always_inline)) inline
/*
 * no extra padding is added by the compiler between struct vars for alignment
*/
#define _ATTR_PACKED \
__attribute__((packed))
/**
 * Checks if arguments are the right types for the specified format.
*/
#define _ATTR_FMT_PRINTF(fmt, args) \
__attribute__((format(printf, fmt, args)))

/* Omit the prologue and epilogue of the specified function. */
#define _ATTR_NAKED \
__attribute__((naked))

/* Specifies minimum alignment for the specified var. */
#define _ATTR_ALIGNED(n) \
__attribute__((aligned(n)))

#endif // __GNUC__

#endif // _DXGMX_ATTRS_H
