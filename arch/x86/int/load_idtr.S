/*
    Copyright (C) Alexandru Olaru.
    Distributed under the MIT license.
*/

.section .text
  .global load_idtr
  .type load_idtr, @function

load_idtr:
  mov  4(%esp), %eax
  lidt (%eax)
  sti
  ret
