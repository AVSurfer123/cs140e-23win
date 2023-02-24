@ hacked code for testing vector base code and testing the
@ difference between the position-independent "generic" method
@ of last lab (that used the ldr instruction) with the faster
@ method of a relative branch (so: no memory load, no cache miss,
@ also, the ldr will commonly = a misprediction).
#include "rpi-asm.h"

.align 5
.globl interrupt_table
interrupt_table:
  ldr pc, =unhandled_reset
  ldr pc, =unhandled_undefined_instruction
  ldr pc, =unhandled_swi
  ldr pc, =unhandled_prefetch_abort
  ldr pc, =unhandled_data_abort
  ldr pc, =unhandled_interrupt
