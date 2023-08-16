/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/elf/elf.h>
#include <dxgmx/elf/elfloader.h>
#include <dxgmx/errno.h>
#include <dxgmx/fs/vfs.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/string.h>
#include <dxgmx/todo.h>
#include <dxgmx/utils/bytes.h>

static int
elfloader_copy_section_from_file(int fd, Elf32Phdr* phdr, Process* actingproc)
{
    off_t off = vfs_lseek(fd, phdr->offset, SEEK_SET, actingproc);
    if (off < 0)
        return -EINVAL;

    /* Read section into buf. */
    ssize_t read = vfs_read(fd, (void*)phdr->vaddr, phdr->filesize, actingproc);
    if (read < 0)
        return read;
    else if ((size_t)read < phdr->filesize)
        return -EINVAL;

    return 0;
}

static int
elfloader_load_from_file32(fd_t fd, Process* actingproc, Process* targetproc)
{
    Elf32Ehdr ehdr32;
    int st = elf_read_ehdr32(fd, actingproc, &ehdr32);
    if (st < 0)
        return st;

    Elf32Phdr* phdrs32 = kmalloc(ehdr32.phnum * sizeof(Elf32Phdr));
    if (!phdrs32)
        return -ENOMEM;

    st = elf_read_phdrs32(fd, actingproc, &ehdr32, phdrs32);
    if (st < 0)
    {
        kfree(phdrs32);
        return st;
    }

    /* Copy sections into memory */
    FOR_EACH_ELEM_IN_DARR (phdrs32, ehdr32.phnum, phdr)
    {
        if (phdr->type != PT_LOAD)
            continue;

        const ptr aligned_start = bytes_align_down64(phdr->vaddr, PAGESIZE);
        const size_t pagespan =
            bytes_align_up64(
                phdr->memsize + (phdr->vaddr - aligned_start), PAGESIZE) /
            PAGESIZE;

        /* Map pages */
        for (size_t i = 0; i < pagespan; ++i)
        {
            ptr vaddr = aligned_start + i * PAGESIZE;
            /* We force PAGE_W because we need to copy the code there, it is
             * cleared later if necessary. */
            u16 flags = (phdr->flags & PAGE_ACCESS_MODE) | PAGE_W;

            st = mm_new_user_page(vaddr, flags, &targetproc->paging_struct);
            if (st < 0)
            {
                kfree(phdrs32);
                return st;
            }
        }

        if (phdr->filesize)
        {
            st = elfloader_copy_section_from_file(fd, phdr, actingproc);
            if (st < 0)
            {
                kfree(phdrs32);
                return st;
            }
        }

        /* If memsize > filesize we pad the rest of the memory with zeros. */
        if (phdr->memsize > phdr->filesize)
        {
            void* start = (void*)(phdr->vaddr + phdr->filesize);
            memset(start, 0, phdr->memsize - phdr->filesize);
        }

        /* Remove the previously possibly forced PAGE_W and set PAGE_USER. */
        for (size_t i = 0; i < pagespan; ++i)
        {
            ptr vaddr = aligned_start + i * PAGESIZE;
            u16 flags = (phdr->flags & PAGE_ACCESS_MODE) | PAGE_USER;

            st = mm_set_page_flags(vaddr, flags, &targetproc->paging_struct);
            ASSERT(st == 0);
        }
    }

    targetproc->inst_ptr = ehdr32.entry;

    kfree(phdrs32);
    return 0;
}

static int elfloader_validate_file(ElfGenericEhdr* ehdr)
{
    if (ehdr->type != ET_EXEC)
        return -ENOEXEC;

    return 0;
}

int elfloader_load_from_file(fd_t fd, Process* actingproc, Process* targetproc)
{
    ElfGenericEhdr ehdr;
    int st = elf_read_generic_ehdr(fd, actingproc, &ehdr);
    if (st < 0)
        return st;

    st = elfloader_validate_file(&ehdr);
    if (st < 0)
        return st;

    /* Map target process' paging struct. */
    mm_load_paging_struct(&targetproc->paging_struct);

    if (ehdr.ident[EI_CLASS] == ELFCLASS32)
        st = elfloader_load_from_file32(fd, actingproc, targetproc);
    else if (ehdr.ident[EI_CLASS] == ELFCLASS64)
        TODO_FATAL();
    else
        st = -EINVAL;

    /* Go back to the acting process' paging struct.
    This is needed for now since if we fail, we will be dropped back into the
    acting process. FIXME: seems hackish*/
    mm_load_paging_struct(&actingproc->paging_struct);

    return st;
}
