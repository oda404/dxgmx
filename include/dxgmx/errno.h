/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_ERRNO_H
#define _DXGMX_ERRNO_H

/* Not permitted */
#define EPERM 1
/* No such file or dir */
#define ENOENT 2
/* No such process */
#define ESRCH 3
/* Interrupted function call */
#define EINTR 4
/* I/O error */
#define EIO 5
/* No such device or address */
#define ENXIO 6
/* Argument list to big */
#define E2BIG 7
/* Exec error */
#define ENOEXEC 8
/* Bad file descriptor */
#define EBADF 9
/* No child process */
#define ECHILD 10
/* Unavailable, try again */
#define EAGAIN 11
/* No memory / cannot allocate */
#define ENOMEM 12
/* No permissions */
#define EACCES 13
/* Bad address */
#define EFAULT 14
/* Not a block device / block device required */
#define ENOTBLK 15
/* Resource is busy */
#define EBUSY 16
/* File exists */
#define EEXIST 17
/* Inproper link */
#define EXDEV 18
/* No such device */
#define ENODEV 19
/* Not a dir */
#define ENOTDIR 20
/* Is a dir */
#define EISDIR 21
/* Invalid argument */
#define EINVAL 22
/* Too many open files in system */
#define ENFILE 23
/* Too many open files in process */
#define EMFILE 24
/* Bad I/O control operation. */
#define ENOTTY 25
/* text file busy */
#define ETXTBSY 26
/* File too big */
#define EFBIG 27
/* No space left on device */
#define ENOSPC 28
/* Invalid seek */
#define ESPIPE 29
/* Readonly FS */
#define EROFS 30
/* Too many links */
#define EMLINK 31
/* Broken pipe */
#define EPIPE 32
/* Math argument out of function domain. */
#define EDOM 33
/* Result too large */
#define ERANGE 34
/* Deadlock avoided */
#define EDEADLK 35
/* File name too long */
#define ENAMETOOLONG 36
/* No locks available. */
#define ENOLCK 37
/* Function/syscall not implemented. */
#define ENOSYS 38

#endif // _DXGMX_ERRNO_H
