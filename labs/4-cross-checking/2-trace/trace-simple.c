// part 1: trace GET32/PUT32 (also: get32, put32).
//
// extension: add a simple log for capturing
//
//  without capture it's unsafe to call during UART operations since infinitely
//  recursive and, also, will mess with the UART state.  
//
//  record the calls in a log (just declare a statically sized array until we get malloc)
//  for later emit by trace_dump()
//
// extension: keep adding more functions!  
//      often it's useful to trace at a higher level, for example, doing 
//      uart_get() vs all the put's/get's it does, since that is easier for
//      a person to descipher.  or, you can do both:
//          -  put a UART_PUTC_START in the log.
//          - record the put/gets as normal.
//          - put a UART_PUTC_STOP in the log.
//  
#include "rpi.h"
#include "trace.h"

// gross that these are not auto-consistent with GET32 in rpi.h
unsigned __wrap_GET32(unsigned addr);
unsigned __real_GET32(unsigned addr);
void __wrap_PUT32(unsigned addr, unsigned val);
void __real_PUT32(unsigned addr, unsigned val);

#define MAX_BUF_LENGTH 4096
unsigned addr_buf[MAX_BUF_LENGTH];
unsigned val_buf[MAX_BUF_LENGTH];
uint8_t type_buf[MAX_BUF_LENGTH];
uint32_t length;
uint8_t buffer_on = 0;

// simple state machine to track what set of options we're called with.
enum {
    TRACE_OFF = 0,
    TRACE_ON,
    TRACE_SKIP   // if running w/o buffering, stop printing trace
};
static int state = TRACE_OFF;

// call these to emit so everyone can compare!
static void emit_put32(unsigned addr, unsigned val) {
    int old_state = state;
    state = TRACE_SKIP;
    printk("TRACE:PUT32(%x)=%x\n", addr, val);
    state = old_state;
}
static void emit_get32(unsigned addr, unsigned val) {
    int old_state = state;
    state = TRACE_SKIP;
    printk("TRACE:GET32(%x)=%x\n", addr, val);
    state = old_state;
}

// start tracing : 
//   <buffer_p> = 0: tells the code to immediately print the 
//                   trace.  
//   <buffer_p> != 0 defers the printing until <trace_stop> is called.
//
//  is an error: if you were already tracing.
void trace_start(int buffer_p) {
    assert(state == TRACE_OFF);
    state = TRACE_ON;
    length = 0;
    buffer_on = buffer_p;
} 

// stop tracing
//  - error: if you were not already tracing.
void trace_stop(void) {
    assert(state == TRACE_ON);
    state = TRACE_OFF;
    // we would print things out if output was being buffered.
    for (int i = 0; i < length; i++) {
        if (type_buf[i]) {
            emit_put32(addr_buf[i], val_buf[i]);
        }
        else {
            emit_get32(addr_buf[i], val_buf[i]);
        }
    }
    if (buffer_on) {
        length = 0;
    }
}


// the linker will change all calls to GET32 to call __wrap_GET32
void __wrap_PUT32(unsigned addr, unsigned val) {
    // XXX: implement this function!
    if (state == TRACE_ON) {
        if (buffer_on) {
            addr_buf[length] = addr;
            val_buf[length] = val;
            type_buf[length] = 1;
            length++;
        }
        else {
            emit_put32(addr, val);
        }
    }
    __real_PUT32(addr, val);
}

// the linker will change all calls to GET32 to call __wrap_GET32
unsigned __wrap_GET32(unsigned addr) {
    unsigned v = 0;
    // implement this function!
    v = __real_GET32(addr);
    if (state == TRACE_ON) {
        if (buffer_on) {
            addr_buf[length] = addr;
            val_buf[length] = v;
            type_buf[length] = 1;
            length++;
        }
        else {
            emit_get32(addr, v);
        }
    }
    return v;
}
