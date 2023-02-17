#include "rpi-asm.h"

@ @ save all the stuff we need, then call the given routine.
@ #define TRAMPOLINE(fn)              \
@   mov sp, #INT_STACK_ADDR;          \
@   push  {r0-r12,lr};                \
@   mov   r0, lr;                     \
@   bl    fn;                         \
@   pop   {r0-r12,lr};                \
@   movs    pc, lr


@ // keep the name of the trampoline private.
@ #define MK_TRAMPOLINE(name, offset, fn)     \
@ .weak name;                  \
@ name:                         \
@     sub   lr, lr, # (offset);               \
@     TRAMPOLINE(fn)

@ MK_TRAMPOLINE(data_abort_trampoline, 8, exception_handler)

.align 5; 
.globl interrupt_table
interrupt_table:
    b unhandled_reset
    b unhandled_undefined_instruction
    b unhandled_swi
    b unhandled_prefetch_abort
    b unhandled_data_abort
    b unhandled_interrupt
    b unhandled_fiq
