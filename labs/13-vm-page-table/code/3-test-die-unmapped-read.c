// test that we die when we do a read to unmmaped memory
#include "vm-ident.h"
#include "libc/bit-support.h"

void vm_test(void) {
    proc.sp_lowest_addr = STACK_ADDR - OneMB;
    proc.dom_id = dom_id;
    proc.pt = vm_ident_mmu_init(1);
    assert(mmu_is_enabled());

    output("should die with a message about an unmmaped read\n");
    volatile uint32_t *p = (void*)(STACK_ADDR +  4*OneMB);
    proc.die_addr = (uint32_t)p;
    get32(p);
    panic("should not be reached\n");
}

void notmain() {
    mmu_be_quiet();
    kmalloc_init_set_start((void*)OneMB, OneMB);
    output("testing we will be killed on a read of unmapped memory\n");
    check_vm_structs();
    vm_test();
}
