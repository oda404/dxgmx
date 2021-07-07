/*
    Copyright (C) Alexandru Olaru.
    Distributed under the MIT license.
*/

#include<dxgmx/x86/int/idt.h>
#include<dxgmx/x86/int/pic.h>
#include<stdbool.h>
#include<stdint.h>

void idt_encode_entry(
#ifdef _X86_
    uint32_t base,
#endif //_X86_
    uint16_t selector,
    uint8_t  flags,
    IDTEntry *entry
)
{
#ifdef _X86_
    entry->base_0_15  = (base & 0xFFFF);
    entry->base_16_31 = (base & 0xFFFF0000) >> 16;
    entry->selector   = selector;
    entry->unused     = 0;
    entry->type       = flags & 0b11111;
    entry->privilege  = flags & (0b11 << 5);
    entry->present    = (bool)(flags & (1 << 7));
#endif // _X86_
}
