/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/errno.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/mem/paging.h>
#include <dxgmx/panic.h>
#include <dxgmx/string.h>

#define KLOGF_PREFIX "paging: "

static int pagingstruct_enlarge_allocated_pages(PagingStruct* ps)
{
    /* just an arbitray number for now */
    size_t newcap = ps->allocated_pages_capacity + 20;

    Page* tmp = krealloc(ps->allocated_pages, newcap * sizeof(Page));
    if (!tmp)
        return -ENOMEM;

    memset(tmp + ps->allocated_pages_capacity, 0, 20 * sizeof(Page));

    ps->allocated_pages = tmp;
    ps->allocated_pages_capacity = newcap;

    return 0;
}

int pagingstruct_init(PagingStruct* ps)
{
    memset(ps, 0, sizeof(PagingStruct));

    int st = pagingstruct_enlarge_allocated_pages(ps);

    return st;
}

int pagingstruct_destroy(PagingStruct* ps)
{
    if (ps->allocated_pages)
    {
        kfree(ps->allocated_pages);
        ps->allocated_pages = NULL;
    }

    ps->allocated_pages_capacity = 0;
    ps->allocated_pages_size = 0;

    return 0;
}

int pagingstruct_track_page(const Page* page, PagingStruct* ps)
{
    if (ps->allocated_pages_size == ps->allocated_pages_capacity)
    {
        int st = pagingstruct_enlarge_allocated_pages(ps);
        if (st < 0)
            return st;
    }

    /* We store tracked pages in ascending order. This helps with freeing them
     * on X86 at least. (Plus the fact that I'm a dipshit my freeing code is
     * trash) */

    size_t idx;
    for (idx = 0; idx < ps->allocated_pages_size; ++idx)
    {
        ptr vaddr = ps->allocated_pages[idx].vaddr;

        if (vaddr == page->vaddr)
            panic("paging: Tried to track a page twice (0x%p).", (void*)vaddr);

        if (vaddr > page->vaddr)
            break;
    }

    for (size_t i = ps->allocated_pages_size; i > idx; --i)
        ps->allocated_pages[i] = ps->allocated_pages[i - 1];

    ps->allocated_pages[idx] = *page;
    ps->allocated_pages_size++;

    return 0;
}
