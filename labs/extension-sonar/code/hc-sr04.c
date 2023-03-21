#include "rpi.h"
#include "hc-sr04.h"

// gpio_read(pin) until either:
//  1. gpio_read(pin) != v ==> return 1.
//  2. <timeout> microseconds have passed ==> return 0
int read_while_eq(int pin, int v, unsigned timeout) {
    unsigned start = timer_get_usec();
    while (timer_get_usec() - start < timeout) {
        if (gpio_read(pin) != v) {
            return 1;
        }
    }
    return 0;
}

// initialize:
//  1. setup the <trigger> and <echo> GPIO pins.
// 	2. init the HC-SR04 (pay attention to time delays here)
// 
// Pay attention to the voltages on:
//    - Vcc
//    - Vout.
//
// Troubleshooting:
// 	1. there are conflicting accounts of what value voltage you
//	need for Vcc.
//	
// 	2. the initial 3 page data sheet you'll find sucks; look for
// 	a longer one. 
//
// The comments on the sparkfun product page might be helpful.
hc_sr04_t hc_sr04_init(unsigned trigger, unsigned echo) {
    hc_sr04_t h = { .trigger = trigger, .echo = echo };
    gpio_set_output(trigger);
    gpio_write(trigger, 0);
    gpio_set_input(echo);
    gpio_set_pulldown(echo);
    delay_cycles(10);
    return h;
}

// get distance.
//	1. do a send (again, pay attention to any needed time 
// 	delays)
//
//	2. measure how long it takes and compute round trip
//	by converting that time to distance using the datasheet
// 	formula
//
// troubleshooting:
//  0. We don't have floating point or integer division.
//
//  1. The pulse can get lost!  Make sure you use the timeout read
//  routine you write.
// 
//	2. readings can be noisy --- you may need to require multiple
//	high (or low) readings before you decide to trust the 
// 	signal.
//
int hc_sr04_get_distance(hc_sr04_t h, unsigned timeout_usec) {
    gpio_write(h.trigger, 1);
    delay_us(10);
    gpio_write(h.trigger, 0);
    if (read_while_eq(h.echo, 0, timeout_usec)) {
        unsigned start = timer_get_usec();
        read_while_eq(h.echo, 1, timeout_usec);
        unsigned time = timer_get_usec() - start;
        return time / 148;
    }
    else {
        return -1;
    }
}
