// Q: does push change sp before writing to it or after?
#include "rpi.h"

// should take a few lines: 
//   - push argument (in r0) onto the stack.
//   - call <after_push> with:
//      - the first argument: sp after you do the push.
//      - second argument: sp before you do the push.

// implement this assembly routine in <asm-checks.S>
uint32_t * push_r4_r12_asm(uint32_t *scratch);


// called with:
//   - <sp_after_push>: value of the <sp> after the push 
//   - <sp_before_push>: value of the <sp> before the push 
void check_push_order(void) {
    uint32_t scratch_mem[64], 
            // pointer to the middle so doesn't matter if up
            // or down
            *p = &scratch_mem[32];

    uint32_t *ret = push_r4_r12_asm(p);
    assert(ret < p);

    // check that regs holds the right values.
    // todo("write the code to check that <ret> holds the expected values.");
    // todo("see caller in <asm-check.S:push_r4_r12_asm>");
    for (int i = 0; i < 9; i++) {
        printk("%d: %u, %u\n", i, ret[i], p[-i-1]);
        assert(ret[i] == i + 4);
    }

    return;

}

void notmain() {
    trace("about to check push order\n");
    check_push_order();
    trace("SUCCESS\n");
}
