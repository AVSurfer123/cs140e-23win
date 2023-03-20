// test of gets_until: enter lines in pi-cat.  if you hit newline will send to the pi
#include "rpi.h"
#include "sw-uart.h"
#include "cycle-count.h"

void notmain(void) {
    // uart_init();
    // enable_cache();

    // use pin 20 for tx, 21 for rx
    sw_uart_t u = sw_uart_init(14, 15, 115200);
    // sw_uart_t u = sw_uart_init(0, 1, 115200);

    unsigned n = 1;

    printk("about to get %d lines: make sure you've started pi-cat\n", n);

    for(int i = 0; i < n; i++) {
        printk("iter=%d, going to do a get of a whole line: type in the pi-cat window!\n", i);

        // this will be on the pi-cat side.
        // sw_uart_printk(&u, "enter a line: ");

        char buf[1024];
        int res = sw_uart_gets(&u, buf, sizeof buf-1);
        // printk("Got %d %d\n", strlen(buf), res);
        assert(strlen(buf) == res);
        // kill newline.
        buf[res-1] = 0;
        printk("SW-UART: got string <%s>\n\n", buf);
    }

    TIME_CYC_PRINT("gpio test", gpio_read(10));
    TIME_CYC_PRINT("print test", printk("stuff\n"));

    clean_reboot();
}
