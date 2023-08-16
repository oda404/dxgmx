
#ifndef _DXGMX_POSIX_FCNTL_DEFS_H
#define _DXGMX_POSIX_FCNTL_DEFS_H

/* Read permissions */
#define O_RDONLY 0x1
/* Write permissions */
#define O_WRONLY 0x2
/* R/W permissions */
#define O_RDWR (O_RDONLY | O_WRONLY)
/* If the file does not exist, create it */
#define O_CREAT 0x8
/* Combined with O_CREAT, this fails if the file already exists  */
#define O_EXCL 0x10
#define O_NOCTTY 0x20
#define O_TRUNC 0x40
/* Set the file offset to the end of the file before each write. */
#define O_APPEND 0x80
#define O_NONBLOCK 0x100
#define O_SYNC 0x2000

#endif // !_DXGMX_POSIX_FCNTL_DEFS_H
