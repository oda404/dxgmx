
#ifndef __DXGMX_PGDIR_H__
#define __DXGMX_PGDIR_H__

#include<stdint.h>
#include<dxgmx/gcc/attrs.h>
#include<dxgmx/paging/pgtable.h>

/*
 * Structure of a page dir entry
 * 
 * |31                                    12|11   9|8                        0|
 * |----------------------------------------+------+--+--+--+--+--+--+--+--+--|
 * | page table base                        |avail.|G |S |0 |A |D |W |U |R |P |
 * |----------------------------------------+------+--+--+--+--+--+--+--+--+--|
*/

/* The table is available in physical memory right now. */
#define _PGTABLE_PRESENT       (1 << 0)
/* The table is read/write. If not set it's read only. */
#define _PGTABLE_RW            (1 << 1)
/* 
 * User/Supervisor bit, controls access to the page table 
 * based on privilege. If set the page table can be accessed by a user 
 * and by the supervisor(kernel), if not only the supervisor can 
 * can access it. All pages that fall under this table 
 * also have the same privilege access level.
 */
#define _PGTABLE_USER_ACCESS   (1 << 2)
/* If set write through is enabled, else write back is enabled. */
#define _PGTABLE_WRITE_THROUGH (1 << 3)
/* Page will not be cached */
#define _PGTABLE_CACHE_DISABLE (1 << 4)
/* 
 * If a page has been accessed (read/written to) this bit
 * will be set.
 */
#define _PGTABLE_ACCESSED      (1 << 5)
/* If set the pages are 4MiB sized, else they are 4KiB. */
#define _PGTABLE_PAGE_SIZE     (1 << 7)

typedef struct 
__ATTR_PACKED S_PageDirectoryEntry
{
    uint32_t table_base: 20;
    uint8_t  avail: 3;
    uint16_t flags: 9;
} PageDirectoryEntry;

typedef struct S_PageDirectory
{
    PageDirectoryEntry entries[1024];
} PageDirectory;

void pgdir_init(PageDirectory *dir);

int pgdir_entry_encode(
    uint32_t base,
    uint8_t flags
);

int pgdir_map_addr(
    uint64_t page_base,
    uint64_t frame_base,
    uint16_t flags,
    PageDirectory *dir
);

#endif // __DXGMX_PGDIR_H__
