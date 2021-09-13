/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

.section .text 
  .global _start
  .type _start, @function

_start:                  # kernel entry point
  movl _kstack_top, %esp  # put stack_top address into the stack_ptr reg

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
