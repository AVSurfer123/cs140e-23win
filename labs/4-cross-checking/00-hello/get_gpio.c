#include "rpi.h"
#include "gpio.h"

#define GPIO_BASE 0x20200000

typedef enum {
    gpio_fsel0 = (GPIO_BASE + 0x00),
    gpio_fsel1 = (GPIO_BASE + 0x04),
    gpio_fsel2 = (GPIO_BASE + 0x08),
    gpio_fsel3 = (GPIO_BASE + 0x0c),
    gpio_set0  = (GPIO_BASE + 0x1C),
    gpio_clr0  = (GPIO_BASE + 0x28),
    gpio_lev0  = (GPIO_BASE + 0x34)
} gpio_pin_t;

void publish_int(uint32_t val) {
    // uint8_t* p = (uint8_t*) &val;
    // for (int i = 0; i < 4; i++) {
    //     rpi_putchar(*p);
    //     p++;
    // }
    printk("%x\n", val);
}

void notmain(void) {
    printk("Start sequence\n");
    uint32_t val;
    val = GET32(gpio_fsel0);
    publish_int(val);
    val = GET32(gpio_fsel1);
    publish_int(val);
    val = GET32(gpio_fsel2);
    publish_int(val);
    val = GET32(gpio_fsel3);
    publish_int(val);
    val = GET32(gpio_set0);
    publish_int(val);
    val = GET32(gpio_clr0);
    publish_int(val);
    val = GET32(gpio_lev0);
    publish_int(val);
    clean_reboot();
}
