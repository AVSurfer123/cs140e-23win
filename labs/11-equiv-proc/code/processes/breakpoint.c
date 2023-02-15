#include <stdint.h>
#include "armv6-debug-impl.h"

// disable mismatch
void brkpt_mismatch_stop(void) {
    cp14_disable();
}

// enable mismatch
void brkpt_mismatch_start(void) {
    cp14_enable();
    // just started, should not be enabled.
    assert(!cp14_bcr0_is_enabled());
    assert(!cp14_wcr0_is_enabled());
}

// set breakpoint on <addr>
void brkpt_mismatch_set(uint32_t addr) {
    cp14_bvr0_set(addr);

    uint32_t b = cp14_bcr0_get();  // set this to the needed bits in wcr0
    b = bits_set(b, 21, 22, 0b10);
    b = bit_clr(b, 20);
    b = bits_set(b, 14, 15, 0);
    b = bits_set(b, 5, 8, 0b1111);
    b = bits_set(b, 3, 4, 0b11);
    b = bits_set(b, 1, 2, 0b11);
    b = bit_set(b, 0);
    cp14_bcr0_set(b);

    assert(cp14_bcr0_is_enabled());
}

// is it a breakpoint fault?
int brkpt_fault_p(void) {
    return was_brkpt_fault();
}
