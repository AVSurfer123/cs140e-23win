// see that you can grow the stack.
#include "vm-ident.h"
#include "libc/bit-support.h"

#define NEW_PT 0x180000

fld_t* new;

void copy_pt(fld_t* pt) {
    new = (fld_t*) NEW_PT;
    for (int i = 0; i < 4096; i++) {
        new[i] = pt[i];
    }
}


void vm_test(void) {
    // setup a simple process structure that tracks where the stack is.
    proc.sp_lowest_addr = STACK_ADDR - OneMB;
    proc.dom_id = dom_id;

    fld_t* pt = vm_ident_mmu_init(1);
    copy_pt(pt);

    fld_t* pte = mmu_map_section(pt, 2*OneMB, 2*OneMB, dom_id);
    pte->nG = 1;

    staff_mmu_sync_pte_mods();

    pte = mmu_map_section(new, 2*OneMB, 3*OneMB, dom_id);
    pte->nG = 1;

    uint32_t shared = 2*OneMB - 16;
    PUT32(shared, 0xbeefdead);
    PUT32(2*OneMB, 0xdeadbeef);

    set_procid_ttbr0(100, 5, new);
    uint32_t val = GET32(shared);
    assert(val == 0xbeefdead);
    val = GET32(2*OneMB);
    assert(val != 0xdeadbeef);

    // do we still have the value after disabling?
    vm_ident_mmu_off();
    assert(!mmu_is_enabled());

    printk("******** success ************\n");
}

void notmain() {
    kmalloc_init_set_start((void*) OneMB, OneMB);
    output("checking that stack gets extended\n");
    vm_test();
}
