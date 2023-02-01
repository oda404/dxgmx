/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_ELF_ELF_H
#define _DXGMX_ELF_ELF_H

/* API for reading and parsing ELF files. Names of structs and struct memebers
 * have been taken from the ELF spec and adapted to fit the dxgmx naming scheme.
 */

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/proc/proc.h>
#include <dxgmx/types.h>

#define EI_MAG0 0
#define EI_MAG1 1
#define EI_MAG2 2
#define EI_MAG3 3
#define EI_CLASS 4
#define EI_DATA 5
#define EI_VERSION 6
#define EI_PAD 7
#define EI_NIDENT 16

#define ELFCLASSNONE 0
#define ELFCLASS32 1
#define ELFCLASS64 2

#define ET_NONE 0
#define ET_REL 1
#define ET_EXEC 2
#define ET_DYN 3
#define ET_CORE 4
#define ET_LOPROC 0xFF00
#define ET_HIPROC 0xFFFF

#define EM_M32 1
#define EM_SPARC 2
#define EM_386 3
#define EM_68K 4
#define EM_88K 5
#define EM_860 7
#define EM_MIPS 8
#define EM_MIPS_RS4_BE 10

#define EV_NONE 0
#define EV_CURRENT 1

#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2
#define PT_INTERP 3
#define PT_NOTE 4
#define PT_SHLIB 5
#define PT_PHDR 6
#define PT_LOPROC 0x70000000
#define PT_HIPROC 0x7fffffff

#define PF_X 1
#define PF_W 2
#define PF_R 4

typedef struct _ATTR_PACKED S_ElfGenericEhdr
{
    u8 ident[EI_NIDENT];
    u16 type;
    u16 machine;
    u32 version;
} ElfGenericEhdr;

/* ELF32 header. */
typedef struct _ATTR_PACKED S_Elf32Ehdr
{
    u8 ident[EI_NIDENT];
    u16 type;
    u16 machine;
    u32 version;

    /* Entry point of the program */
    u32 entry;
    /* Offset of the program header table */
    u32 phoff;
    /* Offset of the section header table */
    u32 shoff;
    /* Processor specific flags */
    u32 flags;
    /* Elf header size */
    u16 ehsize;
    /* Size of one program header */
    u16 phentsize;
    /* Number of program headers. 0 if none. */
    u16 phnum;
    /* Size of one section header */
    u16 shentsize;
    /* Number of section headers */
    u16 shnum;
    /* Incomprehensible, have a good day */
    u16 shstrndx;
} Elf32Ehdr;

typedef struct _ATTR_PACKED S_Elf64Ehdr
{
    u8 ident[EI_NIDENT];
    u16 type;
    u16 machine;
    u32 version;

    /* Entry point of the program */
    u64 entry;
    /* Offset of the program header table */
    u64 phoff;
    /* Offset of the section header table */
    u64 shoff;
    /* Processor specific flags */
    u32 flags;
    /* Elf header size */
    u16 ehsize;
    /* Size of one program header */
    u16 phentsize;
    /* Number of program headers. 0 if none. */
    u16 phnum;
    /* Size of one section header */
    u16 shentsize;
    /* Number of section headers */
    u16 shnum;
    /* Incomprehensible, have a good day */
    u16 shstrndx;
} Elf64Ehdr;

#define ELF_HEADER_MAX_SIZE sizeof(Elf64Hdr)

typedef struct _ATTR_PACKED S_Elf32Phdr
{
    /* Type of this section described by this program header. See PT_* */
    u32 type;
    /* Offset into the file for this segment. */
    u32 offset;
    /* Where to load this segment in memory. */
    u32 vaddr;
    /* Physical address for this segment (unused). */
    u32 paddr;
    /* The size of this segment in the file. */
    u32 filesize;
    /* The size of this segment in memory. */
    u32 memsize;
    /* Access flags for this segment */
    u32 flags;
    /* Alignement of this segments in the file and in the memory. */
    u32 align;
} Elf32Phdr;

/**
 * Read ELF generic header. The ELF generic header is the first few bitsize
 * agnostic bytes. Read this header to figure out if the ELF is 32 or 64 bit.
 *
 * No NULL pointers should be passed to this function.
 * No invalid file descriptors should be passed to this function.
 *
 * 'fd' Open fd of the file.
 * 'proc' Acting process.
 * 'ehdr' Destination generic ELF header.
 *
 * Returns:
 * 0 on success.
 * -EINVAL on invalid arguments.
 * Other values are errors from the vfs.
 */
int elf_read_generic_ehdr(fd_t fd, Process* proc, ElfGenericEhdr* ehdr);

/**
 * Read 32-bit ELF header.
 *
 * No NULL pointers should be passed to this function.
 * No invalid file descriptors should be passed to this function.
 *
 * 'fd' Open fd of the file.
 * 'proc' Acting process.
 * 'ehdr' Destination 32-bit ELF header.
 *
 * Returns:
 * 0 on success.
 * -EINVAL on invalid arguments.
 * Other values are errors from the vfs.
 */
int elf_read_ehdr32(fd_t fd, Process* proc, Elf32Ehdr* ehdr32);

/**
 * Read ELF program headers.
 *
 * No NULL pointers should be passed to this function.
 * No invalid file descriptors should be passed to this function.
 *
 *
 * 'fd' Open fd of the file.
 * 'proc' Acting process.
 * 'hdr32' The already parsed Ehdr32.
 * 'phdrs32' Destination array of however many program headers are to be read
 * (the exact number can be found in hdr32->phnum).
 *
 * Returns:
 * 0 on success.
 * -EINVAL on invalid arguments.
 */
int elf_read_phdrs32(
    fd_t fd, Process* proc, const Elf32Ehdr* hdr32, Elf32Phdr* phdrs32);

#endif // !_DXGMX_ELF_ELF_H
