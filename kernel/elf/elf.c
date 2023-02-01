/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/elf/elf.h>
#include <dxgmx/errno.h>
#include <dxgmx/fs/vfs.h>

#define KLOGF_PREFIX "elf: "

#define GENERIC_EHDR_SIZE sizeof(ElfGenericEhdr)
#define EHDR32_SIZE sizeof(Elf32Ehdr)

static bool elf_valid_magic(const u8 ident[EI_NIDENT])
{
    return ident[EI_MAG0] == 0x7f && ident[EI_MAG1] == 'E' &&
           ident[EI_MAG2] == 'L' && ident[EI_MAG3] == 'F';
}

int elf_read_generic_ehdr(fd_t fd, Process* proc, ElfGenericEhdr* hdr)
{
    off_t off = vfs_lseek(fd, 0, SEEK_SET, proc);
    if (off < 0)
        return off;

    u8 buf[GENERIC_EHDR_SIZE];

    ssize_t read = vfs_read(fd, buf, GENERIC_EHDR_SIZE, proc);
    if (read < 0)
        return read; // An error happened
    else if ((size_t)read < GENERIC_EHDR_SIZE)
        return -EINVAL; // No error occured but the file is invalid

    ElfGenericEhdr* tmpheader = (ElfGenericEhdr*)buf;
    if (!elf_valid_magic(tmpheader->ident))
        return -EINVAL;

    *hdr = *tmpheader;

    return 0;
}

int elf_read_ehdr32(fd_t fd, Process* proc, Elf32Ehdr* hdr32)
{
    /* Make sure we are at the start */
    off_t off = vfs_lseek(fd, 0, SEEK_SET, proc);
    if (off < 0)
        return off;

    u8 buf[EHDR32_SIZE];

    ssize_t read = vfs_read(fd, buf, EHDR32_SIZE, proc);
    if (read < 0)
        return read; // An error happened
    else if ((size_t)read < EHDR32_SIZE)
        return -EINVAL; // No error occured but the file is invalid

    Elf32Ehdr* tmp = (Elf32Ehdr*)buf;
    if (!elf_valid_magic(tmp->ident) || tmp->ident[EI_CLASS] != ELFCLASS32)
        return -EINVAL;

    *hdr32 = *tmp;

    return 0;
}

int elf_read_phdrs32(
    fd_t fd, Process* proc, const Elf32Ehdr* hdr32, Elf32Phdr* phdrs32)
{
    if (sizeof(Elf32Phdr) != hdr32->phentsize)
        return -EINVAL;

    if (!elf_valid_magic(hdr32->ident))
        return -EINVAL;

    for (size_t i = 0; i < hdr32->phnum; ++i)
    {
        const off_t offset = hdr32->phoff + i * sizeof(Elf32Phdr);

        off_t off = vfs_lseek(fd, offset, SEEK_SET, proc);
        if (off < 0)
            return off;

        ssize_t read = vfs_read(fd, &phdrs32[i], sizeof(Elf32Phdr), proc);
        if (read < 0)
            return read;
        else if ((size_t)read < sizeof(Elf32Phdr))
            return -EINVAL;
    }

    return 0;
}
