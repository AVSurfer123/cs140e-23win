#include "regs-save-restore.h"

MK_FN(switchto_user_asm)
    mov lr, r0
    ldm lr, {r0-r14}^
    add lr, lr, #60
    rfe lr
    asm_not_reached()

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
    TRAMPOLINE_FULL_REGS(do_syscall)

@ only handle single step.
.align 5; 
.globl test_restore_handlers
test_restore_handlers:
    b unhandled_reset
    b unhandled_undefined_instruction
    b swi_trampoline
    b unhandled_prefetch_abort 
    b unhandled_data_abort
    b unhandled_interrupt
    b unhandled_fiq

