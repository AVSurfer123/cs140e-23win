// first test do low level setup of everything.
#include "rpi.h"
#include "pinned-vm.h"
#include "mmu.h"
#include "asm-helpers.h"

cp_asm(dfsr, p15, 0, c5, c0, 0)
cp_asm(far, p15, 0, c6, c0, 0)

procmap_t new_map;

void data_abort_vector(uint32_t pc) {
    trace("Exception at pc %x\n", pc);
    int is_load = bit_isset(dfsr_get(), 11) == 0;
    int status = bits_get(dfsr_get(), 0, 3);
    if (is_load) {
        trace("Exception was load with fault type %d at addr %x\n", status, far_get());
    }
    else {
        trace("Exception was store with fault type %d at addr %x\n", status, far_get());
    }
    if (status == 9) {
        domain_access_ctrl_set(dom_perm(0b1110, DOM_client));
    }
    else {
        pin_procmap(&new_map);
    }
}

void prefetch_abort_vector(uint32_t pc) {
    trace("Exception at pc %x\n", pc);
    int is_load = bit_isset(dfsr_get(), 11) == 0;
    int status = bits_get(dfsr_get(), 0, 3);
    if (is_load) {
        trace("Exception was load with fault type %d at addr %x\n", status, far_get());
    }
    else {
        trace("Exception was store with fault type %d at addr %x\n", status, far_get());
    }
    if (status == 9) {
        domain_access_ctrl_set(dom_perm(0b1110, DOM_client));
    }
    else {
        pin_procmap(&new_map);
    }
}

void notmain(void) { 
    enum { OneMB = 1024*1024};
    // map the heap: for lab cksums must be at 0x100000.
    kmalloc_init_set_start((void*)MB, MB);
    assert(!mmu_is_enabled());

    enum { dom_kern = 1,
           // domain for user = 2
           dom_user = 2,
           dom_heap = 3 };          

    // read write the first mb.
    uint32_t user_addr = OneMB * 16;
    assert((user_addr>>12) % 16 == 0);
    uint32_t phys_addr1 = user_addr;
    PUT32(phys_addr1, 0xdeadbeef);

    procmap_t p = procmap_default_mk(dom_kern);
    p.map[4].dom = dom_heap;
    p.dom_ids |= 1 << dom_heap;
    procmap_push(&p, pr_ent_mk(user_addr, MB, MEM_RW, dom_user));

    assert(p.n == MAX_ENT);

    uint32_t unmapped = 3*MB;
    new_map = procmap_default_mk(dom_kern);
    procmap_push(&new_map, pr_ent_mk(unmapped, MB, MEM_RW, dom_heap));

    trace("about to enable\n");
    pin_mmu_on(&p);

    lockdown_print_entries("about to turn on first time");
    assert(mmu_is_enabled());
    trace("MMU is on and working!\n");

    uint32_t heap_addr = MB;
    PUT32(heap_addr, 0xdeadbeef);

    domain_access_ctrl_set(dom_perm(0b110, DOM_client));
    uint32_t x = GET32(heap_addr);
    trace("asid 1 = got: %x\n", x);
    assert(x == 0xdeadbeef);

    domain_access_ctrl_set(dom_perm(0b110, DOM_client));
    PUT32(heap_addr, 0xabc);
    x = GET32(heap_addr);
    trace("asid 1 = got: %x\n", x);
    assert(x == 0xabc);

    PUT32(heap_addr, 0xe12fff1e);
    domain_access_ctrl_set(dom_perm(0b110, DOM_client));
    ((void (*)(void)) heap_addr)();

    PUT32(unmapped, 0xe12fff1e);
    pin_procmap(&p);
    uint32_t y = GET32(unmapped);
    pin_procmap(&p);
    trace("asid 1 = got: %x\n", y);
    assert(y == 0xe12fff1e);
    ((void (*)(void)) unmapped)();
    pin_procmap(&p);

    PUT32(user_addr, 1);
    staff_mmu_disable();
    assert(!mmu_is_enabled());
    trace("MMU is off!\n");
    trace("phys addr1=%x\n", GET32(phys_addr1));
    assert(GET32(phys_addr1) == 1);

    trace("SUCCESS!\n");
}
