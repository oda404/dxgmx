
#define __ASM__
#include <dxgmx/x86/mboot.h>

#define MBOOT_HEADER_FLAGS \
(MBOOT_FLAG_PAGE_ALIGN | MBOOT_FLAG_MEM_INFO)

#define MBOOT_HEADER_CHECKSUM \
-(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

.section .multiboot    # declare header after multiboot std
  .align MBOOT_HEADER_ALIGN
  .long MBOOT_HEADER_MAGIC
  .long MBOOT_HEADER_FLAGS
  .long MBOOT_HEADER_CHECKSUM

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
