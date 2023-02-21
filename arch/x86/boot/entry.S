/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
*/

.global _multiboot_magic
.type _multiboot_magic, @object

.global _multiboot_info_struct_base
.type _multiboot_info_struct_base, @object

.global kinit_stage2
.type kinit_stage2, @function

#define OFFSET 0xC0000000

.section .data

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

# Very small, early stack made for saving critical registers 
# while still in the 'lower half'. It will be replaced once 
# the jump to the 'higher half' is made.
.local _early_stack_top
.local _early_stack_bot
_early_stack_bot:
  .skip 32
_early_stack_top:

.section .init.text, "a"

.type _early_die, @function
_early_die:
  # Clear screen
  movl $1999, %ecx
  movl $0xB8000, %eax
clear:
  movw $0, (%eax)
  addl $2, %eax
  loop clear

  popl %eax           # Get the string address
  movl $0xB8000, %ecx # Address of the VGA text buffer.
loop1:
  movb (%eax), %bl # Put the char in the lower half
  movb $0x0F, %bh  # Black bg, white fg in the higher half.

  movw %bx, (%ecx)

  addl $1, %eax    # Next char
  addl $2, %ecx    # Next slot in the VGA buffer.

  # Exit if NULL terminator has been reached.
  cmp $0, %bl
  jne loop1

1:
  cli
  hlt
  jmp 1b

.size _early_die, . - _early_die

.global _start
.type _start, @function
_start:
  # Clear the interrupt and direction flags
  cli
  cld

  # Setup the early stack for saving any inmportant registers.
  movl $_early_stack_top - OFFSET, %esp 

  # Save eax and ebx, since they hold multiboot stuff.
  pushl %eax
  pushl %ebx

  # If PAE is not supported, die.
  movl $1, %eax
  cpuid
  
  shr $6, %edx
  andl $1, %edx
  cmpl $1, %edx

  pushl $no_pae_string - OFFSET
  jne _early_die
  addl $4, %esp

  # If the kernel is bigger than 1MiB, die, as we don't have
  # enough pages to map it. 
  movl $_kernel_size, %eax
  cmpl $(1024 * 1024), %eax

  pushl $kernel_too_large_string - OFFSET
  jg _early_die
  addl $4, %esp

  movl $boot_pgdir - OFFSET + 1, (boot_pdpt - OFFSET)
  movl $boot_pgdir - OFFSET + 1, (boot_pdpt - OFFSET + 3 * 8)

  # Set up the first two page tables for identity mapping  
  movl $boot_pgtable0 - OFFSET + 3, (boot_pgdir - OFFSET)

  movl $boot_pgtable0 - OFFSET, %eax
  movl $512, %ecx      # loop 512 times.

  movl $3, %edx        # Start from base 0 + 3 (present and r/w bits set)

higher_half_map:
  movl %edx, (%eax)    # set the page frame base
  addl $8, %eax        # increment to the next pte
  addl $4096, %edx     # next page frame base.

  loop higher_half_map # loop until %ecx is 0
higher_half_map_end:

  # Restore eax and ebx
  popl %ebx
  popl %eax

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

  # Jump to the higher half.
  lea higher_half, %ecx
  jmp *%ecx

higher_half:
  movl _kernel_stack_top, %esp # setup the actual stack.
  movl $0, %ebp                # NULL ebp marks the end of a stack backtrace

  # clear eflags
  pushl $0
  popf

  # Zero out the first pdpt entry, unmapping the 0-1GiB range,
  # leaving only the higher half map. 
  movl $0, (boot_pdpt)

  # Flush the PDPT.
  movl %cr3, %ecx
  movl %ecx, %cr3

  movl %ebx, _multiboot_info_struct_base
  movl %eax, _multiboot_magic

  call kinit_stage2           # less go

.size _start, . - _start
