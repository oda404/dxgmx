/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#define __ASM__

.section .multiboot
multiboot_header:
#if defined(__MBOOT2__)

  #include <dxgmx/x86/mboot2.h>

  .align  MBOOT2_HEADER_ALIGN
  .long   MBOOT2_HEADER_MAGIC
  .long   MBOOT2_ARCH_I386
  .long   multiboot_header_end - multiboot_header
  .long   -(MBOOT2_HEADER_MAGIC + MBOOT2_ARCH_I386 + (multiboot_header_end - multiboot_header))
  
  .align  MBOOT2_HEADER_ALIGN
  .short  MBOOT2_HEADER_TAG_END
  .short  0
  .long   8
  
#elif defined(__MBOOT__)

  #include <dxgmx/x86/mboot.h>
  #define FLAGS MBOOT_FLAG_PAGE_ALIGN | MBOOT_FLAG_MEM_INFO

  .align  MBOOT_HEADER_ALIGN
  .long   MBOOT_HEADER_MAGIC
  .long   FLAGS
  .long   -(MBOOT_HEADER_MAGIC + FLAGS)

#else

  # standalone, and i don t support that yet
  oopsie

#endif // defined(__MBOOT2__)
multiboot_header_end:

.section .bss          # stack for the initial kernel thread
  .align 16
stack_bottom:
  .skip 16384
stack_top:

.section .text 
  .global _start
  .type _start, @function

_start:                  # kernel entry point
  movl $stack_top, %esp  # put stack_top address into the stack_ptr reg

  # clear eflags
  pushl $0               # push 0 on the stack
  popf                   # pop 0 off the stack and store it into EFLAGS

  # prepare args for kmain
  pushl %ebx             # push the multiboot struct addr on the stack
  pushl %eax             # push the magic number on the stack
  
  call kmain             # less go

1:                       # halt if kmain shits itself
  cli
  hlt
  jmp 1b                 # loop back in case of nmis

.size _start, . - _start
