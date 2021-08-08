/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#define _ASM

#include<dxgmx/x86/multiboot.h>

.set FLAGS, MULTIBOOT_FLAG_MOD_PAGE_ALIGN | MULTIBOOT_FLAG_MEM_INFO | MULTIBOOT_FLAG_VIDEO
.set CHECKSUM, -(MULTIBOOT_HEADER_MAGIC + FLAGS)
.set HEADER_ADDR,     0
.set LOAD_ADDR,       0
.set LOAD_END_ADDR,   0
.set BSS_END_ADDR,    0
.set ENTRY_ADDR,      0
.set VIDEO_MODE_TYPE, 1
.set VIDEO_WIDTH,     0
.set VIDEO_HEIGHT,    0
.set VIDEO_DEPTH,     0

.section .multiboot
  .long MULTIBOOT_HEADER_MAGIC
  .long FLAGS
  .long CHECKSUM
  .long HEADER_ADDR
  .long LOAD_ADDR
  .long LOAD_END_ADDR
  .long BSS_END_ADDR
  .long ENTRY_ADDR
  .long VIDEO_MODE_TYPE
  .long VIDEO_WIDTH
  .long VIDEO_HEIGHT
  .long VIDEO_DEPTH

.section .bss          # stack for the initial kernel thread
  .global _kstack_top
  .global _kstack_bot
  .align 16
_kstack_bot:
  .skip 16384
_kstack_top:

.section .text 
  .global _start
  .type _start, @function

_start:                  # kernel entry point
  movl $_kstack_top, %esp  # put stack_top address into the stack_ptr reg

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
