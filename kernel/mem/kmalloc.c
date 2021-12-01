
#include<dxgmx/kmalloc.h>
#include<dxgmx/string.h>

static ptr g_heap_start;
static ptr g_heap_size;

static ptr g_current_heap_start;

typedef struct
S_KMallocBlockMeta
{
#define KMALLOC_BLOCK_SIG 0xBADA110C
    size_t sig;
    size_t blocksize;
} KMallocBlockMeta;

int kmalloc_init(ptr heap_start, size_t heap_size)
{
    memset((void *)heap_start, 0, heap_size);

    g_heap_start = heap_start;
    g_heap_size = heap_size;

    g_current_heap_start = heap_start;

    return 0;
}

void *kmalloc(size_t size)
{
    /* dumbass kmalloc implementation. */
    size_t actualsize = size + sizeof(KMallocBlockMeta);

    if(g_current_heap_start + actualsize > g_heap_start + g_heap_size)
        return NULL;

    *(KMallocBlockMeta *)g_current_heap_start = (KMallocBlockMeta) {
        .sig = KMALLOC_BLOCK_SIG,
        .blocksize = size
    };

    g_current_heap_start += actualsize + sizeof(KMallocBlockMeta);
    return (void *)(g_current_heap_start - actualsize);
}

void kfree(void *addr)
{
    KMallocBlockMeta *meta = (KMallocBlockMeta *)(addr - sizeof(KMallocBlockMeta));

    if(meta->sig != KMALLOC_BLOCK_SIG)
        return;

    memset(meta, 0, sizeof(KMallocBlockMeta));
}

void *krealloc(void *addr, size_t size)
{
    (void)addr;
    (void)size;
    return NULL;
}
