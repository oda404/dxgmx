
  .set ALIGN,    0x0              # align modules on page boundaries
  .set MEMINFO,  0x1              # provide a memory map
  .set FLAGS,    ALIGN | MEMINFO  # multiboot flag
  .set MAGIC,    0x1BADB002       # magic number for header id
  .set CHECKSUM, -(MAGIC + FLAGS) # checksum for multiboot


  .section .multiboot    # declare header after multiboot std
  .align 4
  .long MAGIC
  .long FLAGS
  .long CHECKSUM

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
  
  call kmain

dead_loop:               # halt if kmain shits itself
  cli
  hlt
  jmp dead_loop          # loop back in case of nmi

.size _start, . - _start
