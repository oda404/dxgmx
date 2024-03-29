/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/compiler_attrs.h>
#include <dxgmx/errno.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/ksyms.h>
#include <dxgmx/stdlib.h>
#include <dxgmx/string.h>

#define KLOGF_PREFIX "ksyms: "

#define KSYMS_SIZE (32 * KIB)

typedef struct S_KernelSymbol
{
    ptr addr;
    const char* str;
    size_t strlen;
} KernelSymbol;

extern u8 _ksyms_section_base[];
extern u8 _ksyms_section_end[];

_ATTR_SECTION(".ksyms") static u8 g_ksyms[KSYMS_SIZE];
static unsigned long g_ksyms_entries_cnt = 0;
static bool g_ksyms_available = false;
static KernelSymbol* g_ksyms_table = NULL;

_INIT int ksyms_load()
{
    /**
     * The .ksyms section structure:
     *  - 0:8 characters: 1BADADD4\n
     *  - 9:17 the number of symbols in the format of %08X\n.
     */
    if (memcmp(g_ksyms, "1BADADD4\n", 9))
    {
        KLOGF(
            ERR,
            ".ksyms section doesn't contain the expected magic number. Kernel symbols will not be available!");
        return -EINVAL;
    }

    char buf[9] = {0};
    /* Jump over the magic number */
    memcpy(buf, g_ksyms + 9, 8);

    int st = strtoul(buf, NULL, 16, &g_ksyms_entries_cnt);
    if (st < 0 || *(g_ksyms + 17) != '\n')
    {
        KLOGF(
            ERR,
            "Could not parse the symbol entries count. Kernel symbols will not be available!");
        return st;
    }

    g_ksyms_table = kmalloc(g_ksyms_entries_cnt * sizeof(KernelSymbol));
    if (!g_ksyms_table)
    {
        KLOGF(
            ERR,
            "Could not allocate the kernel symbol table. Kernel symbols will not be available!");
        return -ENOMEM;
    }
    memset(g_ksyms_table, 0, g_ksyms_entries_cnt * sizeof(KernelSymbol));

    const char* ksyms = (const char*)(g_ksyms + 18);
    for (size_t i = 0; i < g_ksyms_entries_cnt; ++i)
    {
        memcpy(buf, ksyms, 8);

        /* Parse the symbol address. */
        unsigned long addr;
        st = strtoul(buf, NULL, 16, &addr);
        if (st < 0)
        {
            KLOGF(
                ERR,
                "Found invalid symbol entry. Kernel symbols will not be available!");
            return st;
        }

        /* Jump over the address, two whitespaces and the type,
        pointing directly at the symbol name. */
        ksyms += 11;

        /* The symbol entry's end is denoted by a '\n' */
        size_t len = 0;
        while (ksyms[len] != '\n')
            ++len;

        g_ksyms_table[i].addr = addr;
        g_ksyms_table[i].str = ksyms;
        g_ksyms_table[i].strlen = len;

        ksyms += len + 1;
    }
    g_ksyms_available = true;

    return 0;
}

size_t ksyms_get_symbol_name(ptr addr, ptr* offset, char* name, size_t n)
{
    if (!g_ksyms_available || !name)
        return 0;

    KernelSymbol* ksym = NULL;

    {
        i32 l = 0, r = g_ksyms_entries_cnt - 1;
        while (l <= r)
        {
            i32 m = (l + r) / 2;
            ksym = &g_ksyms_table[m];

            if (ksym->addr < addr)
                l = m + 1;
            else if (ksym->addr > addr)
                r = m - 1;
            else
                break;
        }
    }

    if (ksym->addr > addr && (ptr)ksym > (ptr)g_ksyms_table)
        --ksym;

    size_t wr = ksym->strlen < n ? ksym->strlen : n;
    strncpy(name, ksym->str, wr);

    if (offset)
        *offset = addr - ksym->addr;
    return wr;
}
