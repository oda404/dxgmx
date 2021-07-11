
#ifndef _DXGMX_PAGING_PAGEDIR_H
#define _DXGMX_PAGING_PAGEDIR_H

#include<stdint.h>
#include<dxgmx/attrs.h>
#include<dxgmx/paging/pagetable.h>

/*
 * Structure of a page directory entry
 * 
 * |31                                    12|11   9|8                        0|
 * |----------------------------------------+------+--+--+--+--+--+--+--+--+--|
 * | page table base                        |unused|G |S |0 |A |D |W |U |R |P |
 * |----------------------------------------+------+--+--+--+--+--+--+--+--+--|
*/

typedef struct 
_ATTR_PACKED S_PageDirectoryEntry
{
    /* This is a 4KiB aligned address to a PageTable. */
    uint32_t table_base: 20;
    /* Free for use :) */
    uint8_t unused: 3;
    uint8_t ignored: 1;
    /* Set if page_size is 4MiB */
    uint8_t page_size: 1;
    /* Set to 0 */
    uint8_t zero: 1;
    /* Set if page has been accessed. */
    uint8_t accessed: 1;
    /* Set if caching is disabled. */
    uint8_t cache_disabled: 1;
    /* Set if write_through is enabled. */
    uint8_t write_through: 1;
    /* Set if page can be accessed by users. */
    uint8_t user_access: 1;
    uint8_t rw: 1;
    /* Set if page is present in physical memory */
    uint8_t present: 1;
} PageDirectoryEntry;

typedef struct S_PageDirectory
{
    PageDirectoryEntry entries[1024];
} PageDirectory;

void pagedir_init(PageDirectory *dir);

int pagedir_add(
    uint64_t page_base,
    uint64_t frame_base,
    uint16_t flags,
    PageDirectory *dir,
    PageTable tables[1024]
);

#endif // _DXGMX_PAGING_PAGEDIR_H
