/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#define S_IXOTH 1
#define S_IWOTH 2
#define S_IROTH 4
#define S_IRWXO (S_IXOTH | S_IWOTH | S_IROTH)

#define S_IXGRP 0x8
#define S_IWGRP 0x10
#define S_IRGRP 0x20
#define S_IRWXG (S_IXGRP | S_IWGRP | S_IRGRP)

#define S_IXUSR 0x40
#define S_IWUSR 0x80
#define S_IRUSR 0x100
#define S_IRWXU (S_IXUSR | S_IWUSR | S_IRUSR)

#define S_ISVTX 0x200
#define S_ISGID 0x400
#define S_ISUID 0x800

#define S_IREAD S_IRUSR
#define S_IWRITE S_IWUSR
#define S_IEXEC S_IXUSR

#define S_IFIFO 0x1000
#define S_IFCHR 0x2000
#define S_IFDIR 0x4000
#define S_IFREG 0x8000
#define S_IFBLK 0x6000
#define S_IFLNK 0xa000
#define S_IFSOCK 0xc000
#define S_IFMT 0xf000

#endif // !_SYS_STAT_H
