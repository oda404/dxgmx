
#ifndef __DXGMX_GCC_ATTRS_H__
#define __DXGMX_GCC_ATTRS_H__

/* attr may not be used */
#define __ATTR_MAYBE_UNUSED    __attribute__((unused))
/* function is pure (does not modify program state nor any passed ptrs) */
#define __ATTR_PURE            __attribute__((pure))
/*
 * function's return value will not be affected by the program's state, 
 * passing in the same args will always result the same return value
*/
#define __ATTR_CONST           __attribute__((const))
/* 
 * ptrs passed to the function will be checked by the compiler 
 * to be non-null, if null a warning will be issued
*/
#define __ATTR_NONNULL(...)    __attribute__((nonnull(__VA_ARGS__)))
/* used on a malloc like function
 * ptr returned by the function cannot alias any other valid pointer
 * opt arg1: dealloc is a suitable deallocation function for ptrs obtained through said malloc-like function
*/
#define __ATTR_MALLOC(...)     __attribute__((malloc, __VA_ARGS__))
/* used on a malloc-like function
 * arg1: int which represents the size of the memory block
 * pointed to by the returned ptr
*/
#define __ATTR_ALLOC_SIZE(...) __attribute__((alloc_size(__VA_ARGS__)))

#endif // __DXGMX_GCC_ATTRS_H__
