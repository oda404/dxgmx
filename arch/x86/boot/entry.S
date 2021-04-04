
#define __ASM__
#include <dxgmx/x86/mboot.h>

.section .multiboot
multiboot_header:
#if defined(__MBOOT2__)

  .align  MBOOT2_HEADER_ALIGN
  .long   MBOOT2_HEADER_MAGIC
  .long   MBOOT2_ARCH_I386
  .long   multiboot_header_end - multiboot_header
  .long   -(MBOOT2_HEADER_MAGIC + MBOOT2_ARCH_I386 + (multiboot_header_end - multiboot_header))

#elif defined(__MBOOT__)

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

  pushl $0
  popf

  pushl %ebx
  pushl %eax
  
  call kmain

dead_loop:               # halt if kmain shits itself
  cli
  hlt
  jmp dead_loop          # loop back in case of nmis

.size _start, . - _start
