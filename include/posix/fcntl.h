/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _FCNTL_H
#define _FCNTL_H

#define O_RDONLY 0x1
#define O_WRONLY 0x2
#define O_RDWR (O_RDONLY | O_WRONLY)
#define O_ACCMODE (O_RDONLY | O_WRONLY)
#define O_EXEC 0x4
#define O_CREAT 0x8
#define O_EXCL 0x10
#define O_NOCTTY 0x20
#define O_TRUNC 0x40
#define O_APPEND 0x80
#define O_NONBLOCK 0x100
#define O_DIRECTORY 0x200
#define O_CLOEXEC 0x800
#define O_DIRECT 0x1000
#define O_SYNC 0x2000

#endif // !_FCNTL_H
