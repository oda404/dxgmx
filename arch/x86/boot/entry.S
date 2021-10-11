/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

.section .text 

.global _kstack_top
.type _kstack_top, @object

.global _multiboot_magic
.type _multiboot_magic, @object

.global _multiboot_info_struct_base
.type _multiboot_info_struct_base, @object

.global kstack_get_top
.type kstack_get_top, @function

.global kmain
.type kmain, @function

.global _start
.type _start, @function

_start:                 # kernel entry point
  movl %eax, %ecx       # save the value of eax in ecx
  call kstack_get_top
  movl %eax, %esp       # put stack_top address into the stack_ptr reg
  movl %ecx, %eax       # restore the original value of eax

  # clear eflags
  pushl $0              # push 0 on the stack
  popf                  # pop 0 off the stack and store it into EFLAGS

  movl %ebx, _multiboot_info_struct_base
  movl %eax, _multiboot_magic
  
  call kmain             # less go

1:                       # halt if kmain shits itself
  cli
  hlt
  jmp 1b                 # loop back in case of nmis

.size _start, . - _start
