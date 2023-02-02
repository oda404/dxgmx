/**
 * Copyright 2022 Alexandru Olaru.
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

        /* Calculate stuff */
        /* Let's hope phdr->align is right */
        const ptr aligned_start = bytes_align_down64(phdr->vaddr, phdr->align);

        const size_t pagespan =
            bytes_align_up64(phdr->memsize, PAGESIZE) / PAGESIZE;

        /* Map pages */
        for (size_t i = 0; i < pagespan; ++i)
        {
            st = mm_new_user_page(
                aligned_start + i * PAGESIZE, 0, &targetproc->paging_struct);

            if (st < 0)
            {
                kfree(phdrs32);
                return st;
            }
        }

        /* Allocate stuff for reading */
        u8* buf = kmalloc(phdr->filesize);
        if (!buf)
        {
            kfree(phdrs32);
            return -ENOMEM;
        }

        /* Seek to where we need to be */
        off_t off = vfs_lseek(fd, phdr->offset, SEEK_SET, actingproc);
        if (off < 0)
        {
            kfree(buf);
            kfree(phdrs32);
            return -EINVAL;
        }

        /* Read section into buf. */
        ssize_t read = vfs_read(fd, buf, phdr->filesize, actingproc);
        if (read < 0)
        {
            kfree(buf);
            kfree(phdrs32);
            return read;
        }
        else if ((size_t)read < phdr->filesize)
        {
            kfree(buf);
            kfree(phdrs32);
            return -EINVAL;
        }

        /* Rawdoggin it */
        memcpy((void*)phdr->vaddr, buf, phdr->filesize);

        /* If memsize > filesize we pad the rest of the memory with zeros. */
        if (phdr->memsize > phdr->filesize)
        {
            void* start = (void*)(phdr->vaddr + phdr->filesize);
            size_t size = phdr->memsize - phdr->filesize;
            memset(start, 0, size);
        }

        kfree(buf);
    }

    targetproc->inst_ptr = ehdr32.entry;

    kfree(phdrs32);
    return 0;
}

int elfloader_validate_file(fd_t fd, Process* actingproc)
{
    ElfGenericEhdr ehdr;
    int st = elf_read_generic_ehdr(fd, actingproc, &ehdr);
    if (st < 0)
        return st;

    if (ehdr.type != ET_EXEC)
        return -ENOEXEC;

    return 0;
}

int elfloader_load_from_file(fd_t fd, Process* actingproc, Process* targetproc)
{
    ElfGenericEhdr ehdr;
    int st = elf_read_generic_ehdr(fd, actingproc, &ehdr);
    if (st < 0)
        return st;

    /* Map target process' paging struct. */
    mm_load_paging_struct(&targetproc->paging_struct);

    if (ehdr.ident[EI_CLASS] == ELFCLASS32)
    {
        return elfloader_load_from_file32(fd, actingproc, targetproc);
    }
    else if (ehdr.ident[EI_CLASS] == ELFCLASS64)
    {
        TODO_FATAL();
    }

    /* Go back to the acting process' paging struct.
    This is needed for now since if we fail, we will be dropped back into the
    acting process. FIXME: seems hackish*/
    mm_load_paging_struct(&actingproc->paging_struct);

    return -EINVAL;
}
