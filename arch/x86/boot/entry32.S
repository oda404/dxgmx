/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#define _ASM
#include <dxgmx/generated/kconfig.h>
#include <dxgmx/x86/gdt.h>

.global kinit_stage2
.type kinit_stage2, @function

#define OFFSET 0xC0000000

#ifdef CONFIG_64BIT
#define SIZE_T .quad
#define SIZE_T_BYTES 8
#else
#define SIZE_T .long
#define SIZE_T_BYTES 4
#endif

# GDT. Previously defined and initiated in gdt.c, we moved it here for long mode
# since long mode requires some setup in the gdt and once we get to C we should
# already be running in long mode. (if supported)
# If this structure is to be changed the relevant defines in dxgmx/x86/gdt.h are also to be updated!
.section .ro_post_init
.global ___boot_gdt
___boot_gdt:
null:
  .quad 0

code0:
  .short 0xFFFF # limit lo
  .byte 0 # base lo
  .byte 0 # base lo
  .byte 0 # base lo
  .byte (1 << 1) | (1 << 3) | (1 << 4) | (1 << 7) # flags (rw, exec, code_or_data, present, w/ dpl 0)
  .byte 0xF | (1 << 6) | (1 << 7) # 0xF for limit hi, 32bit, granularity 4K
  .byte 0 # base hi

data0:
  .short 0xFFFF # limit lo
  .byte 0 # base lo
  .byte 0 # base lo
  .byte 0 # base lo
  .byte (1 << 1) | (1 << 4) | (1 << 7) # flags (rw, code_or_data, present, w/ dpl 0)
  .byte 0xF | (1 << 6) | (1 << 7) # 0xF for limit hi, 32bit, granularity 4K
  .byte 0 # base hi

code3:
  .short 0xFFFF # limit lo
  .byte 0 # base lo
  .byte 0 # base lo
  .byte 0 # base lo
  .byte (1 << 1) | (1 << 3) | (1 << 4) | (3 << 5) | (1 << 7) # flags (rw, exec, code_or_data, present, w/ dpl 0)
  .byte 0xF | (1 << 6) | (1 << 7) # 0xF for limit hi, 32bit, granularity 4K
  .byte 0 # base hi

data3:
  .short 0xFFFF # limit lo
  .byte 0 # base lo
  .byte 0 # base lo
  .byte 0 # base lo
  .byte (1 << 1) | (1 << 4) | (3 << 5) | (1 << 7) # flags (rw, code_or_data, present, w/ dpl 0)
  .byte 0xF | (1 << 6) | (1 << 7) # 0xF for limit hi, 32bit, granularity 4K
  .byte 0 # base hi

tss:
  .quad 0 # tss is setup later in C

.global ___boot_gdt_size
___boot_gdt_size: 
  SIZE_T . - ___boot_gdt

.local g_gdtr
g_gdtr:
  .short . - ___boot_gdt - 1 - SIZE_T_BYTES # SIZE_T_BYTES from ___boot_gdt_size
  .long ___boot_gdt

.section .rodata

kernel_too_large_string:
  .asciz "Kernel has grown too big. Not proceeding."

no_pae_string:
  .asciz "CPU doesn't support PAE. Not proceeding."

.section .bss
# The PDPT for the mandatory PAE paging.
.align 4096
boot_pdpt:
  .fill 4, 8, 0

.align 4096
boot_pgdir:
  .fill 512, 8, 0

# Used for identity mapping the first 2MiB of memory and also
# mapping the same 2MiB to to the 'higher half'.
.align 4096
boot_pgtable0:
  .fill 512, 8, 0

# The kernel stack
.section .kstack, "aw", @nobits
.align 16
.global _kernel_stack_bot
.global _kernel_stack_top
_kernel_stack_bot:
  .skip CONFIG_KSTACK_SIZE
_kernel_stack_top:

.section .init.text, "a"

.type _early_die, @function
_early_die:
1:
  cli
  hlt
  jmp 1b

.global _start
.type _start, @function
_start:
  # Clear the interrupt and direction flags
  cli
  cld

  # Setup stack
  movl $_kernel_stack_top - OFFSET, %esp

  # Check for and save bootloader variables
#if defined(CONFIG_MULTIBOOT_BOOTLOADER)
  cmpl $0x2BADB002, %eax
  jne 1f
  movl %eax, ___multiboot_magic - OFFSET
  movl %ebx, ___multiboot_struct_pa - OFFSET
1:
#endif
  # Even if we don't find a configured bootloader spec, 
  # we keep going. kinit_arch will panic later because of this.

  # If PAE is not supported, die.
  movl $1, %eax
  cpuid
  shr $6, %edx
  andl $1, %edx
  cmpl $1, %edx
  pushl $no_pae_string - OFFSET
  jne _early_die
  addl $4, %esp

  # If the kernel is bigger than 1MiB, die, as we don't have enough pages to map it. 
  movl $_kernel_size, %eax
  cmpl $(1024 * 1024), %eax
  pushl $kernel_too_large_string - OFFSET
  jg _early_die
  addl $4, %esp

  # Set up the first two page tables for identity mapping  
  movl $boot_pgdir - OFFSET + 1, (boot_pdpt - OFFSET)
  movl $boot_pgdir - OFFSET + 1, (boot_pdpt - OFFSET + 3 * 8)
  movl $boot_pgtable0 - OFFSET + 3, (boot_pgdir - OFFSET)
  movl $boot_pgtable0 - OFFSET, %eax
  movl $512, %ecx      # loop 512 times.
  movl $3, %edx        # Start from base 0 + 3 (present and r/w bits set)

1:
  movl %edx, (%eax)    # set the page frame base
  addl $8, %eax        # increment to the next pte
  addl $4096, %edx     # next page frame base.

  loop 1b # loop until %ecx is 0

  # Set the PAE bit of CR4.
  movl %cr4, %ecx
  orl $(1 << 5), %ecx
  movl %ecx, %cr4

  # Load CR3 with the address of the PDPT.
  movl $boot_pdpt - OFFSET, %ecx
  movl %ecx, %cr3

  # Set the PE and WP bits of CR3, enabling paging.
  movl %cr0, %ecx
  orl $0x80010000, %ecx
  movl %ecx, %cr0

  # Right now we are executing code in the initial kernel mapping (somewhere ~1MiB)
  # We need to jump to a higher half mapped function (~3GiB)
  lea higher_half, %ecx
  jmp *%ecx

higher_half:
  # Add the kernel map offset to the stack pointer, now that we're in the higher half
  add $OFFSET, %esp

  # NULL ebp marks the end of a stack backtrace
  movl $0, %ebp

  # Zero out the first pdpt entry, unmapping the 0-1GiB range,
  # leaving only the higher half map. 
  movl $0, (boot_pdpt)

  # Flush the PDPT.
  movl %cr3, %ecx
  movl %ecx, %cr3
  
  # Now that we're higher half mapped we load the gdt because the gdt's 
  # address can now be accessed without playing games.
  # We could've loaded it earlier and just switched it again right before
  # unmapping the first GiB but I don't see any benifit in doing that
  lgdt g_gdtr
  ljmp $GDT_KERNEL_CS, $reload_gdt_segments
reload_gdt_segments:
  movw $GDT_KERNEL_DS, %ax
  movw %ax, %ds
  movw %ax, %es
  movw %ax, %fs
  movw %ax, %gs
  movw %ax, %ss

  # clear eflags
  pushl $0
  popf

  # Note that interrupts are still off.

  # less go
  call kinit_stage2
