/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_UUID_H
#define _DXGMX_UUID_H

#include <dxgmx/types.h>

/* The lenght of an UUID string without the NULL terminator. */
#define UUID_LENGTH 36
/* The 'printf' format of an UUID string. */
#define UUID_FMT "%08X-%04X-%04X-%04X-%012X"

/**
 * @brief Formats an UUID string.
 *
 * @param a 0-4 bytes of the UUID
 * @param b 4-6 bytes of the UUID
 * @param c 6-8 bytes of the UUID
 * @param d 8-10 bytes of the UUID
 * @param e 10-16 bytes of the UUID
 * @param dest Destination string.
 *
 * @return If the value is negative it represents an errno,
 * else the number of characters written.
 */
int uuid_format(u32 a, u16 b, u16 c, u16 d, u64 e, char* dest);

/**
 * @brief Generates an UUID string.
 *
 * @param dest Destination string.
 *
 * @return If the value is negative it represents an errno,
 * else the number of characters written.
 */
int uuid_gen(char* dest);

#endif // !_DXGMX_UUID_H
