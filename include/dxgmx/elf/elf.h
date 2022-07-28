/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_ELF_ELF_H
#define _DXGMX_ELF_ELF_H

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/types.h>

#define ELF_ARCH_UNSPECIFIED 0x0
#define ELF_ARCH_SPARC 0x2
#define ELF_ARCH_X86 0x3
#define ELF_ARCH_MIPS 0x8
#define ELF_ARCH_PPC 0x14
#define ELF_ARCH_ARM 0x28
#define ELF_ARCH_SUPERH 0x2A
#define ELF_ARCH_IA64 0x32
#define ELF_ARCH_X86_64 0x3E
#define ELF_ARCH_AARCH64 0xB7
#define ELF_ARCH_RISCV 0xF3

#define ELF_BITS_32 1
#define ELF_BITS_64 2

#define ELF_ENDIANNESS_LITTLE 1
#define ELF_ENDIANNESS_BIG 2

#define ET_NONE 0
#define ET_REL 1
#define ET_EXEC 2
#define ET_DYN 3
#define ET_CORE 4
#define ET_LOOS 0xFE00
#define ET_HIOS 0xFEFF
#define ET_LOPROC 0xFF00
#define ET_HIPROC 0xFFFF

#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2
#define PT_INTERP 3
#define PT_NOTE 4
#define PT_SHLIB 5
#define PT_PHDR 6
#define PT_LOPROC 0x70000000
#define PT_HIPROC 0x7fffffff

#define ELF_MAGIC 0x464C457F

#define ELF_GENERIC_HEADER                                                     \
    struct _ATTR_PACKED                                                        \
    {                                                                          \
        /* 0x7F, 'E', 'L', 'F' */                                              \
        u32 magic;                                                             \
        /* See ELF_BITS_* */                                                   \
        u8 bits;                                                               \
        /* See ELF_ENDIANNESS_* */                                             \
        u8 endianness;                                                         \
        /* ??? */                                                              \
        u8 header_version;                                                     \
        /* ??? */                                                              \
        u8 abi;                                                                \
        u64 unused;                                                            \
        /* The file type. See ELF_TYPE_* */                                    \
        u16 type;                                                              \
        /* The architecture. See ELF_ARCH_* */                                 \
        u16 arch;                                                              \
        /* ??? */                                                              \
        u32 elf_version;                                                       \
    }

typedef ELF_GENERIC_HEADER ElfGenericHdr;

typedef struct _ATTR_PACKED S_Elf32Hdr
{
    ELF_GENERIC_HEADER;
    u32 entry_point;
    u32 phdr_table;
    u32 shdr_table;
    u32 flags;
    u16 header_size;
    u16 phdr_table_entry_size;
    u16 phdr_table_entry_count;
    u16 shdr_table_entry_size;
    u16 shdr_table_entry_count;
    u16 section_names_idx;
} Elf32Hdr;

typedef struct _ATTR_PACKED S_Elf64Hdr
{
    ELF_GENERIC_HEADER;
    u64 entry_point;
    u64 phdr_table;
    u64 shdr_table;
    u32 flags;
    u16 header_size;
    u16 phdr_table_entry_size;
    u16 phdr_table_entry_count;
    u16 shdr_table_entry_size;
    u16 shdr_table_entry_count;
    u16 section_names_idx;
} Elf64Hdr;

#define ELF_HEADER_MAX_SIZE sizeof(Elf64Hdr)

typedef struct _ATTR_PACKED S_Elf32ProgramHdr
{
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
    u32 flags;
    /* Alignement of this segments in the file and in the memory. */
    u32 alignment;
} Elf32Phdr;

int elf_read_generic_hdr(int fd, ElfGenericHdr* hdr);
int elf_read_hdr32(int fd, Elf32Hdr* hdr);
int elf_read_phdrs32(
    int fd, const Elf32Hdr* hdr, Elf32Phdr* phdrs);

#endif // !_DXGMX_ELF_ELF_H
