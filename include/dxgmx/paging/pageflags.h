
#ifndef _DXGMX_PAGING_PAGEFLAGS_H
#define _DXGMX_PAGING_PAGEFLAGS_H

typedef enum E_PageFlags
{
    /**
     *  The page is present physical memory. 
     */
    PAGEFLAG_PRESENT        = (1 << 0),
    /**
     *  The page is read/write. 
     */
    PAGEFLAG_RW             = (1 << 1),
    /**
     * The page can be accessed by hypervisor/user.
     */
    PAGEFLAG_USER_ACCESS    = (1 << 2),
    /**
     * The page has write through enabled.
     */
    PAGEFLAG_WRITE_THROUGH  = (1 << 3),
    /**
     * The page has caching disabled.
     */
    PAGEFLAG_CACHE_DISABLED = (1 << 4),
    /**
     * Set if the page has been read/written to.
     */
    PAGEFLAG_ACCESSED       = (1 << 5),
    /**
     * Set if the page has been written to.
     */
    PAGEFLAG_DIRTY          = (1 << 6),
    /** 
     *  The page should be kept in the TLB between CR3 resets.
     */
    PAGEFLAG_GLOBAL         = (1 << 8)
} PageFlags;

#endif //_DXGMX_PAGING_PAGEFLAGS_H
