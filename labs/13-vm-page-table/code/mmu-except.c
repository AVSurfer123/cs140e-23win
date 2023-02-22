#include "vm-ident.h"
#include "libc/bit-support.h"
#include "armv6-debug.h"
#include "rpi.h"

volatile struct proc_state proc;

coproc_mk(dfsr, p15, 0, c5, c0, 0)
coproc_mk(far, p15, 0, c6, c0, 0)

coproc_mk(ifsr, p15, 0, c5, c0, 1)
coproc_mk(ifar, p15, 0, c6, c0, 2)

// this is called by reboot: we turn off mmu so that things work.
void reboot_callout(void) {
    if(mmu_is_enabled())
        mmu_disable();
}

void prefetch_abort_vector(unsigned lr) {
    int status = bits_get(cp15_ifsr_get(), 0, 3);
    uint32_t addr = cp15_ifar_get();
    if (addr == proc.die_addr) {
        // Printing for tests
        if (status == 0b101) {
            printk("ERROR: attempting to run unmapped addr %x (reason=101)\n", addr);
        }
        else if (status == 0b1101) {
            printk("ERROR: attempting to run addr %x with no permissions (reason=1101)\n", addr);
        }
        clean_reboot();
    }
}

void data_abort_vector(unsigned pc) {
    trace("Exception at pc %x\n", pc);
    int is_load = bit_isset(cp15_dfsr_get(), 11) == 0;
    int status = bits_get(cp15_dfsr_get(), 0, 3);
    uint32_t addr = cp15_far_get();
    if (addr == proc.die_addr) {
        // Printing for tests
        if (status == 0b101) {
            if (is_load) {
                printk("ERROR: attempting to load unmapped addr %x: dying (reason=101)\n", addr);
            }
            else {
                printk("ERROR: attempting to store unmapped addr %x: dying (reason=101)\n", addr);
            }
        }
        else if (status == 0b1101) {
            if (is_load) {
                printk("ERROR: attempting to load addr %x permission error: dying (reason=1101)\n", addr);
            }
            else {
                printk("ERROR: attempting to store addr %x permission error: dying (reason=1101)\n", addr);
            }
        }
        clean_reboot();
    }
    uint32_t va = bits_clr(addr, 0, 19);
    if (status == 0b101) {
        mmu_map_section(proc.pt, va, va, proc.dom_id);
        staff_mmu_sync_pte_mods();
    }
    else if (status == 0b1101) {
        mmu_mark_sec_rw(proc.pt, va, 1);
        staff_mmu_sync_pte_mods();
    }
    proc.fault_count++;
}
