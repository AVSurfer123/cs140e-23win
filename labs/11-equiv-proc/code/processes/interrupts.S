#include "rpi-asm.h"

#define TRAMPOLINE_FULL_REGS(fn)    \
  sub sp, sp, #68;                          \
  stm sp, {r0-r14}^;                        \
  str lr, [sp, #60];                        \
  mov   r2, lr;                             \
  mrs   r1, spsr;                           \
  str r1, [sp, #64];                        \
  mov   r0, sp;                             \
  bl    fn;                                 \
  ldr r1, [sp, #64];                        \
  msr   spsr, r1;                           \
  ldm   sp, {r0-r14}^;                      \
  add sp, sp, #60;                          \
  rfe sp;                                   \
  asm_not_reached()

@ save all the registers in ascending order.
swi_trampoline:
    mov sp, #INT_STACK_ADDR
    TRAMPOLINE_FULL_REGS(do_syscall)

single_step_trampoline:
    mov sp, #INT_STACK_ADDR
    sub   lr, lr, #4
    TRAMPOLINE_FULL_REGS(single_step_full)

interrupt_trampoline:
    mov sp, #INT_STACK_ADDR
    sub   lr, lr, #4
    TRAMPOLINE_FULL_REGS(timer_interrupt)
    
@ pointer to the array passed in <r0>
@ use rfe to invoke it.
MK_FN(rfe_asm)
    rfe r0
    asm_not_reached();

MK_FN(switchto_user_asm)
    mov lr, r0
    ldm lr, {r0-r14}^
    add lr, lr, #60
    rfe lr
    asm_not_reached()

@ only handle single step.
.align 5; 
.globl interrupt_table
interrupt_table:
    b unhandled_reset
    b unhandled_undefined_instruction
    b swi_trampoline
    b single_step_trampoline
    b unhandled_data_abort
    b interrupt_trampoline
    b interrupt_trampoline


