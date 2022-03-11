/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_ATTRS_H
#define _DXGMX_ATTRS_H

#include <dxgmx/compiler_attrs.h>

#undef LIKELY
#undef UNLIKELY

#ifdef __has_builtin
#if __has_builtin(__builtin_expect)
/* https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html#index-g_t_005f_005fbuiltin_005fexpect-4159
 */
#define LIKELY(x) __builtin_expect(x, 1)
/* https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html#index-g_t_005f_005fbuiltin_005fexpect-4159
 */
#define UNLIKELY(x) __builtin_expect(x, 0)
#endif
#endif // __has_builtin

#ifndef LIKELY
#define LIKELY(x) x
#endif

#ifndef UNLIKELY
#define UNLIKELY(x) x
#endif

/* Signals that the function is only used during initialization
and can be discarded once that is done. */
#define _INIT _ATTR_COLD _ATTR_SECTION(".init.text")

/* The variable will become read only at the end of the
kinit_stage1 function. */
#define _RO_POST_INIT _ATTR_SECTION(".ro_post_init")

#endif // _DXGMX_ATTRS_H
