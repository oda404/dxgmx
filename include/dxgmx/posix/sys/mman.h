/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _SYS_MMAN_H
#define _SYS_MMAN_H

/* Data can be read */
#define PROT_READ (1 << 0)
/* Data can be written */
#define PROT_WRITE (1 << 1)
/* Data can be executed */
#define PROT_EXEC (1 << 2)
/* Data can not be accessed */
#define PROT_NONE (1 << 3)

#define MAP_SHARED (1 << 0)
#define MAP_PRIVATE (1 << 1)
#define MAP_FIXED (1 << 2)

#define MAP_FAILED ((void*)0)

#endif // !_SYS_MMAN_H
