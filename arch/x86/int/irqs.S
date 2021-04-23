/*
    Copyright (C) Alexandru Olaru.
    Distributed under the MIT license.
*/

.section .text
  .global irq0_entry, irq1_entry, irq2_entry
  .global irq3_entry, irq4_entry, irq5_entry
  .global irq6_entry, irq7_entry, irq8_entry
  .global irq9_entry, irq10_entry, irq11_entry
  .global irq12_entry, irq13_entry, irq14_entry, irq15_entry
  .type irq0_entry,  @function
  .type irq1_entry,  @function
  .type irq2_entry,  @function
  .type irq3_entry,  @function
  .type irq4_entry,  @function
  .type irq5_entry,  @function
  .type irq6_entry,  @function
  .type irq7_entry,  @function
  .type irq8_entry,  @function
  .type irq9_entry,  @function
  .type irq10_entry, @function
  .type irq11_entry, @function
  .type irq12_entry, @function
  .type irq13_entry, @function
  .type irq14_entry, @function
  .type irq15_entry, @function

irq0_entry:
  pusha
  call irq0_handler
  popa
  iret

irq1_entry:
  pusha
  call irq1_handler
  popa
  iret

irq2_entry:
  pusha
  call irq2_handler
  popa
  iret

irq3_entry:
  pusha
  call irq3_handler
  popa
  iret

irq4_entry:
  pusha
  call irq4_handler
  popa
  iret

irq5_entry:
  pusha
  call irq5_handler
  popa
  iret

irq6_entry:
  pusha
  call irq6_handler
  popa
  iret

irq7_entry:
  pusha
  call irq7_handler
  popa
  iret

irq8_entry:
  pusha
  call irq8_handler
  popa
  iret

irq9_entry:
  pusha
  call irq9_handler
  popa
  iret

irq10_entry:
  pusha
  call irq10_handler
  popa
  iret

irq11_entry:
  pusha
  call irq11_handler
  popa
  iret

irq12_entry:
  pusha
  call irq12_handler
  popa
  iret

irq13_entry:
  pusha
  call irq13_handler
  popa
  iret

irq14_entry:
  pusha
  call irq14_handler
  popa
  iret

irq15_entry:
  pusha
  call irq15_handler
  popa
  iret
