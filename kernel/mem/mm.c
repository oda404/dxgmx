/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/errno.h>
#include <dxgmx/kboot.h>
#include <dxgmx/kimg.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/mem/falloc.h>
#include <dxgmx/mem/mem_limits.h>
#include <dxgmx/mem/mm.h>
#include <dxgmx/mem/pagefault.h>
#include <dxgmx/proc/procm.h>
#include <dxgmx/string.h>
#include <dxgmx/utils/bytes.h>

#define KLOGF_PREFIX "mm: "

#define BOOTSTRAP_HEAP_SIZE (200 * KIB)
static _ATTR_ALIGNED(PAGESIZE) u8
    g_bootstrap_heap_space[BOOTSTRAP_HEAP_SIZE] = {0};
static Heap g_bootstrap_heap = {
    .vaddr = (ptr)g_bootstrap_heap_space,
    .pagespan = BOOTSTRAP_HEAP_SIZE / PAGESIZE};

static MemoryRegionMap* g_sys_mregmap;
static PagingStruct g_kernel_paging_struct;

extern void mm_setup_paging_arch(PagingStruct* kps);
extern void pagefault_setup_arch();

extern ptr mm_va2pa_arch(ptr va, const PagingStruct* ps);

extern int mm_init_paging_struct_arch(PagingStruct* ps);
extern void mm_destroy_paging_struct_arch(PagingStruct* ps);
extern void mm_load_paging_struct_arch(const PagingStruct* ps);
extern int mm_map_kernel_into_paging_struct_arch(PagingStruct* ps);

extern int mm_map_page_arch(ptr vaddr, ptr paddr, u16 flags, PagingStruct* ps);
extern int mm_set_page_flags_arch(ptr vaddr, u16 flags, PagingStruct* ps);
extern int mm_rm_page_flags_arch(ptr vaddr, u16 flags, PagingStruct* ps);

static _INIT void mm_setup_sys_mregmap()
{
    g_sys_mregmap = ___kboot_info.mregmap;

    KLOGF(INFO, "Available memory provided by firmware:");
    FOR_EACH_MEM_REGION (region, g_sys_mregmap)
    {
        KLOGF(
            INFO,
            "  0x%p-0x%p",
            (void*)region->start,
            (void*)(region->start + region->size));
    }

    /* Remove the 1st MiB */
    mregmap_rm_reg(0, MIB, g_sys_mregmap);
    KLOGF(INFO, "Reserved 0x%p-0x%p", (void*)0, (void*)(MIB));

    /* Remove the kernel image. */
    mregmap_rm_reg(kimg_paddr(), kimg_size(), g_sys_mregmap);
    KLOGF(
        INFO,
        "Reserved 0x%p-0x%p",
        (void*)kimg_paddr(),
        (void*)(kimg_paddr() + kimg_size()));

    mregmap_align_regs(PAGESIZE, g_sys_mregmap);

    KLOGF(INFO, "Available memory before modules:");
    FOR_EACH_MEM_REGION (region, g_sys_mregmap)
    {
        char unit[4];
        KLOGF(
            INFO,
            "  0x%p-0x%p (%.2f%s)",
            (void*)region->start,
            (void*)(region->start + region->size),
            (double)bytes_to_human_readable(region->size, unit),
            unit);
    }
}

static _INIT void mm_setup_dma(ptr dma_start, size_t dma_size)
{
    Heap dmaheap = {.vaddr = dma_start, .pagespan = dma_size / PAGESIZE};
    {
        char unit[4];
        KLOGF(
            DEBUG,
            "dma start: 0x%p, max size: %d%s.",
            (void*)dmaheap.vaddr,
            (size_t)bytes_to_human_readable(dmaheap.pagespan * PAGESIZE, unit),
            unit);
    }

    procm_get_kernel_proc()->dma_heap = dmaheap;
}

static _INIT void mm_setup_kmalloc(ptr kheap_start, size_t kheap_size)
{
    Heap kheap = {.vaddr = kheap_start, .pagespan = kheap_size / PAGESIZE};
    {
        char unit[4];
        KLOGF(
            DEBUG,
            "kheap start: 0x%p, max size: %d%s.",
            (void*)kheap.vaddr,
            (size_t)bytes_to_human_readable(kheap.pagespan * PAGESIZE, unit),
            unit);
    }

    if (kmalloc_init() < 0)
        panic("Failed to initialize kmalloc!");

    /* The heap situation (at least on x86 PAE since that's the only config
     * this bitch runs on) is kind of tricky. The kernel is mapped in increments
     * of 2MiBs since that's how much a page table holds. The kernel heap starts
     * right after the kernel image + the other stuff the bootloader put there
     * and spans off to the end of the virtual address space minus dma. That
     * most often means that the kernel heap is not completely mapped, as should
     * be the case. Depending on the kmalloc implementation this may or may not
     * cause problems. If kmalloc returns an unmapped address and in the attempt
     * of trying to map said address kmalloc is called once again and returns
     * yet another unmapped address we're kind of dead. The situation gets even
     * harder if the kernel occupies almost or even all of the 2x MiBs it
     * has, in such cases kmalloc is left with no available space to even try to
     * bootstrap itself. A solution I spent a weekend on trying to get to work
     * but ultimately failed is switch to an "emergency heap" in case of 2
     * nested kernel heap pagefaults. This doesn't quite click into place as I
     * would've liked it to, because mm_map_page_arch is called twice (nested)
     * and can overwrite a pagetable it just allocated using the
     * emergency heap. Right now we use a bootstrap heap that is baked into the
     * kernel, that way it's guaranteed to be mapped and we consume the entirety
     * of it in mapping part of the actual heap. This has problems, but it's
     * better than what we had before
     */

    int bootstrap_heap_id = kmalloc_register_heap(g_bootstrap_heap);
    if (bootstrap_heap_id < 0)
        panic("kmalloc failed to register emergency heap!");

    kmalloc_use_heap(bootstrap_heap_id);

    int kheap_id = kmalloc_register_heap(kheap);
    if (kheap_id < 0)
        panic("kmalloc failed to register kernel heap!");

    // kmalloc_use_heap(kheap_id);
}

static _INIT int mm_setup_vm_allocation()
{
    /* FIXME: an initramfs may be coming which means this shit will not fly. We
     * should get this info during early boot */
    const ptr kernel_vend = kimg_vaddr() + kimg_size();

    /* sanity check */
    ASSERT(kernel_vend % PAGESIZE == 0);

    /* Make sure the kernel heap is zeroed out. We always start mapping
    memory from 0 to where ever the kernel end is, in 2 MiB increments
    (whole Page tables). Let's say the kernel starts at 1 MiB and is around
    ~324 KiB the rest of ~700 KiB are considered kernel heap, and are still
    mapped. Note that new kernel heap pages allocated when a pagefault
    happens are automatically zeroed out. */
    /* FIXME: The 2MiB increment may not always be true. Find a way to get this
     * information */
    const size_t sz = bytes_align_up64(kernel_vend, 0x200'000) - kernel_vend;
    memset((void*)kernel_vend, 0, sz);

    /* Mark these pages in falloc */
    for (size_t i = 0; i < sz / PAGESIZE; ++i)
    {
        ptr paddr = mm_kva2pa(kernel_vend + i * PAGESIZE);
        falloc_one_at(paddr);
    }

    const size_t kernel_hole_size = MEM_KERNEL_SPACE_VA_END - kernel_vend;

    const ptr kheap_start = kernel_vend;
    /* MAYBE FIXME: We might waste a bit of memory here */
    const size_t kheap_size = bytes_align_down64(
        kernel_hole_size / 100 * MEM_KERNEL_HEAP_PERC, PAGESIZE);
    mm_setup_kmalloc(kernel_vend, kheap_size);

    procm_spawn_kernel_proc();

    const ptr dma_start = kheap_start + kheap_size;
    /* MAYBE FIXME: We might waste a bit of memory here */
    const size_t dma_size = bytes_align_down64(
        kernel_hole_size / 100 * MEM_KERNEL_DMA_PERC, PAGESIZE);
    mm_setup_dma(dma_start, dma_size);
    return 0;
}

static _INIT void mm_enforce_ksections_perms()
{
    PagingStruct* kps = &g_kernel_paging_struct;

    FOR_EACH_PAGE (kimg_bootloader_start(), kimg_bootloader_end(), page)
        mm_rm_page_flags(page, PAGE_PRESENT, kps);

    FOR_EACH_PAGE (kimg_text_start(), kimg_text_end(), page)
        mm_rm_page_flags(page, PAGE_W, kps);

    FOR_EACH_PAGE (kimg_syscalls_start(), kimg_syscalls_end(), page)
        mm_rm_page_flags(page, PAGE_WX, kps);

    FOR_EACH_PAGE (kimg_useraccess_start(), kimg_useraccess_end(), page)
        mm_rm_page_flags(page, PAGE_W, kps);

    FOR_EACH_PAGE (kimg_module_start(), kimg_module_end(), page)
        mm_rm_page_flags(page, PAGE_X, kps);

    /* Can't write to init, which is just text */
    FOR_EACH_PAGE (kimg_init_start(), kimg_init_end(), page)
        mm_rm_page_flags(page, PAGE_W, kps);

    FOR_EACH_PAGE (kimg_data_start(), kimg_data_end(), page)
        mm_rm_page_flags(page, PAGE_X, kps);

    /* For now only disable execution. */
    FOR_EACH_PAGE (kimg_ro_postinit_start(), kimg_ro_postinit_end(), page)
        mm_rm_page_flags(page, PAGE_X, kps);

    /* Rodata section can't be written to or executed from. */
    FOR_EACH_PAGE (kimg_rodata_start(), kimg_rodata_end(), page)
        mm_rm_page_flags(page, PAGE_WX, kps);

    /* Can't execute from bss. */
    FOR_EACH_PAGE (kimg_bss_start(), kimg_bss_end(), page)
        mm_rm_page_flags(page, PAGE_X, kps);

    /* Can't write or execute from ksyms */
    FOR_EACH_PAGE (kimg_ksyms_start(), kimg_ksyms_end(), page)
        mm_rm_page_flags(page, PAGE_WX, kps);
}

_INIT void mm_init()
{
    /* Sets up any stuff that wasn't done in entry.S */
    mm_setup_paging_arch(&g_kernel_paging_struct);

    /* Start getting pagefaults. */
    pagefault_setup_arch();

    /* Enforce permissions for all kernel sections. */
    mm_enforce_ksections_perms();

    /* Setup the system memory map */
    mm_setup_sys_mregmap();

    /* Initialize the frame allocator, depends on the system memory map. */
    falloc_init(g_sys_mregmap);

    mm_setup_vm_allocation();
}

int mm_init_paging_struct(PagingStruct* ps)
{
    int st = pagingstruct_init(ps);
    if (st < 0)
        return st;

    st = mm_init_paging_struct_arch(ps);
    if (st < 0)
    {
        pagingstruct_destroy(ps);
        return st;
    }

    return 0;
}

void mm_destroy_paging_struct(PagingStruct* ps)
{
    mm_destroy_paging_struct_arch(ps);
    pagingstruct_destroy(ps);
}

int mm_map_kernel_into_paging_struct(PagingStruct* ps)
{
    return mm_map_kernel_into_paging_struct_arch(ps);
}

void mm_load_paging_struct(const PagingStruct* ps)
{
    mm_load_paging_struct_arch(ps);
}

void mm_load_kernel_paging_struct()
{
    mm_load_paging_struct_arch(&g_kernel_paging_struct);
}

PagingStruct* mm_get_kernel_paging_struct()
{
    return &g_kernel_paging_struct;
}

ptr mm_kpa2va(ptr pa)
{
    return pa + kimg_map_offset();
}

int mm_map_page(ptr vaddr, ptr paddr, u16 flags, PagingStruct* ps)
{
    ASSERT(vaddr % PAGESIZE == 0);
    ASSERT(paddr % PAGESIZE == 0);

    return mm_map_page_arch(vaddr, paddr, flags, ps);
}

int mm_new_user_page(ptr vaddr, u16 flags, PagingStruct* ps)
{
    ASSERT(vaddr % PAGESIZE == 0);

    ptr paddr = falloc_one_user();
    if (!paddr)
        return -ENOMEM;

    int st = mm_map_page_arch(vaddr, paddr, flags | PAGE_USER, ps);
    if (st < 0)
    {
        ffree_one(paddr);
        return st;
    }

    Page page = {.vaddr = vaddr};
    st = pagingstruct_track_page(&page, ps);
    if (st < 0)
        ASSERT_NOT_HIT(); // FIXME: can't be fucked.

    return st;
}

int mm_set_page_flags(ptr va, u16 flags, PagingStruct* ps)
{
    return mm_set_page_flags_arch(va, flags, ps);
}

int mm_rm_page_flags(ptr va, u16 flags, PagingStruct* ps)
{
    return mm_rm_page_flags_arch(va, flags, ps);
}

ptr mm_va2pa(ptr va, const PagingStruct* ps)
{
    return mm_va2pa_arch(va, ps);
}

ptr mm_kva2pa(ptr va)
{
    return va - kimg_map_offset();
}
