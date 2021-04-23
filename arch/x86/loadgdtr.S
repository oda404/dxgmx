/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

.section .text
  .global __asm_loadgdt
  .type __asm_loadgdt, @function

__asm_loadgdt:
  /* 
   * esp + 4 is the first arg, a ptr 
   * to the gdt descriptor.
   * move that into eax and 'dereference' it
   * for lgdt
  */
  mov  4(%esp), %eax
  lgdt (%eax)

  # CS reg can't be modified directly
  # so we ljmp 0x8, $reload_segs
  # in the end CS will be 0x8 and IP will be $reload_segs
  # and we continue execution
  # 0x8 points at the new code selector
  ljmp $0x8,  $reload_segs

reload_segs:
  # reload data segment regs
  # this can be modified directly
  # 0x10 points to the new data selector
  movw $0x10, %ax
  movw %ax,   %ds
  movw %ax,   %es
  movw %ax,   %fs
  movw %ax,   %gs
  movw %ax,   %ss
  ret
