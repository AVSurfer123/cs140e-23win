/* 
 */
#include "rpi.h"
#include "asm-helpers.h"
#include "cpsr-util.h"
#include "breakpoint.h"
#include "vector-base.h"
#include "fast-hash32.h"
#include "rpi-armtimer.h"
#include "timer-interrupt.h"

#include "proc.h"

void mov_ident(void);

volatile static uint32_t timer_cnt = 0;

static int verbose_p = 0;
static void dump_regs(uint32_t regs[17]) {
    if(!verbose_p)
        return;

    for(unsigned i = 0; i < 17; i++)
        if(regs[i])
            output("reg[%d]=%x\n", i, regs[i]);
}

int do_syscall(uint32_t regs[17]) {
    int sysno = regs[0];
    assert(sysno == 0);

    if(verbose_p) {
        output("------------------------------------------------\n");
        output("in syscall: sysno=%d\n", sysno);
        dump_regs(regs);
    }
    proc_t *p = curproc_get();
    trace("FINAL: pid=%d: total instructions=%d, reg-hash=%x\n", 
            p->pid,
            p->inst_cnt, 
            p->reg_hash);

    if(p->expected_hash) {
        if(p->expected_hash != p->reg_hash) {
            panic("hash mismatch: expected %x, have %x\n", 
                p->expected_hash, p->reg_hash);
        }
    }
    output("Num timer calls %u\n", timer_cnt);
    trace("PC %x, CPSR %x\n", regs[15], regs[16]);

    proc_exit();
    proc_run_one();
    not_reached();
}

void timer_interrupt(uint32_t regs[17]) {
    dev_barrier();
    unsigned pending = GET32(IRQ_basic_pending);
    // printk("Pending: %d\n", pending);

    // if this isn't true, could be a GPU interrupt (as discussed in Broadcom):
    // just return.  [confusing, since we didn't enable!]
    // use p 113,114 of broadcom.
    if((pending & RPI_BASIC_ARM_TIMER_IRQ) == 0)
        return;

    timer_cnt++;
    // trace("PC %x, CPSR %x\n", regs[15], regs[16]);
    // trace("IN TIMER INT:%u\n", timer_get_usec());
    // output("here\n");

    /* 
     * Clear the ARM Timer interrupt - it's the only interrupt we have
     * enabled, so we don't have to work out which interrupt source
     * caused us to interrupt 
     *
     * Q: what happens, exactly, if we delete?
     */
    PUT32(arm_timer_IRQClear, 1);
    dev_barrier();
}

void single_step_full(uint32_t regs[17]) {
    uint32_t pc = regs[15];

    if(!brkpt_fault_p())
        panic("pc=%x: is not a breakpoint fault??\n", pc);

    proc_t *p = curproc_get();
    // todo("compute reg hash and instructions.\n");
    p->inst_cnt++;
    p->reg_hash = fast_hash_inc32((void*) regs, 17 * 4, p->reg_hash);

    if(verbose_p)  {
        output("------------------------------------------------\n");
        output("cnt=%d: single step pc=%x, hash=%x\n", p->inst_cnt, pc, p->reg_hash);
        dump_regs(regs);
    }

    uint32_t spsr = spsr_get();
    if(mode_get(spsr) != USER_MODE)
        panic("pc=%x: is not at user level: <%s>?\n", pc, mode_str(spsr));
    if(regs[16] != spsr)
        panic("saved spsr %x not equal to <%x>?\n", regs[16], spsr);

    timer_interrupt(regs);
    
    proc_save(regs);
    proc_run_one();
}


void hello(void) {
    output("hello world from pid=%d\n", curproc_get()->pid);
    // int val = 0;
    // for (int i = 0; i < 1000; i++) {
    //     val += i;
    // }
    // output("Value at end %d\n", val);
    asm volatile("swi 1" ::: "memory");
}

void notmain(void) {
    kmalloc_init_set_start((void*)(1024*1024), 1024*1024);

    // turn off system interrupts
    cpsr_int_disable();
    //  BCM2835 manual, section 7.5 , 112
    dev_barrier();
    PUT32(Disable_IRQs_1, 0xffffffff);
    PUT32(Disable_IRQs_2, 0xffffffff);
    dev_barrier();
    extern uint32_t interrupt_table[];
    vector_base_set(interrupt_table);
    
    timer_interrupt_init(1000);
    dev_barrier();

    brkpt_mismatch_start(); 
    dev_barrier();

    // start global interrupts.
    cpsr_int_enable();


    void nop_1(void);

    output("about to check that swi test works\n");
    enum { N = 10 };
    for(int i = 0; i < N; i++) {
        // proc_fork_nostack(mov_ident, 0xcd6e5626);
        // proc_fork_nostack(nop_1, 0xbfde46be);
        proc_fork_stack(hello, 0);
    }
    proc_run_one();

}
