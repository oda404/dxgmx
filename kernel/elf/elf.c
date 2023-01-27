/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/elf/elf.h>
#include <dxgmx/errno.h>
#include <dxgmx/fs/vfs.h>
#include <dxgmx/klog.h>
#include <dxgmx/string.h>

#define KLOGF_PREFIX "elf: "

int elf_read_generic_hdr(int fd, Process* proc, ElfGenericHdr* hdr)
{
    if (fd < 0 || !hdr)
        return -EINVAL;

    off_t off = vfs_lseek(fd, 0, SEEK_SET, proc);
    if (off < 0)
        return off;

#define HEADER_SIZE ((ssize_t)sizeof(ElfGenericHdr))

    u8 buf[HEADER_SIZE] = {0};

    ssize_t read = vfs_read(fd, buf, HEADER_SIZE, proc);
    if (read < 0)
        return read; // An error happened
    else if (read < HEADER_SIZE)
        return -EINVAL; // No error occured but the file is invalid

    ElfGenericHdr* tmpheader = (ElfGenericHdr*)buf;
    if (tmpheader->magic != ELF_MAGIC)
        return -EINVAL;

    *hdr = *tmpheader;

#undef HEADER_SIZE

    return 0;
}

int elf_read_hdr32(int fd, Process* proc, Elf32Hdr* hdr)
{
    if (fd < 0 || !hdr)
        return -EINVAL;

    off_t off = vfs_lseek(fd, 0, SEEK_SET, proc);
    if (off < 0)
        return off;

#define HEADER_SIZE ((ssize_t)sizeof(Elf32Hdr))

    u8 buf[HEADER_SIZE] = {0};

    ssize_t read = vfs_read(fd, buf, HEADER_SIZE, proc);
    if (read < 0)
        return read; // An error happened
    else if (read < HEADER_SIZE)
        return -EINVAL; // No error occured but the file is invalid

    Elf32Hdr* tmpheader = (Elf32Hdr*)buf;
    if (tmpheader->magic != ELF_MAGIC || tmpheader->bits != ELF_BITS_32)
        return -EINVAL;

    *hdr = *tmpheader;

#undef HEADER_SIZE

    return 0;
}

int elf_read_phdrs32(
    int fd, Process* proc, const Elf32Hdr* hdr, Elf32Phdr* phdrs)
{
    if (fd < 0 || !hdr || !phdrs)
        return -EINVAL;

    if (sizeof(Elf32Phdr) != hdr->phdr_table_entry_size)
        return -EINVAL;

    if (hdr->magic != ELF_MAGIC)
        return -EINVAL;

    for (size_t i = 0; i < hdr->phdr_table_entry_count; ++i)
    {
        const off_t offset = hdr->phdr_table + i * sizeof(Elf32Phdr);

        off_t off = vfs_lseek(fd, offset, SEEK_SET, proc);
        if (off < 0)
            return off;

        ssize_t read = vfs_read(fd, &phdrs[i], sizeof(Elf32Phdr), proc);
        if (read < 0)
            return read;
        else if (read < (ssize_t)sizeof(Elf32Phdr))
            return -EINVAL;
    }

    return 0;
}
