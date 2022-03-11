/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_CTYPE_H
#define _DXGMX_CTYPE_H

#include <dxgmx/compiler_attrs.h>

int isdigit(int c) _ATTR_CONST;

int isspace(int c) _ATTR_CONST;

int isxdigit(int c) _ATTR_CONST;

#endif // _DXGMX_CTYPE_H