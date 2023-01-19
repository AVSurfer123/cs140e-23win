// part2: test GPIO input.
//   - if we get a signal on the input pin 21, turn the LED on.
#include "rpi.h"

void notmain(void) {
    const int led = 20;
    const int input = 21;
    const int led2 = 17;

    gpio_set_output(led);
    gpio_set_output(led2);
    gpio_set_input(input);
    gpio_set_pullup(input);

    unsigned val = 0;
    
    while(1) { 
        // could also do: 
        //  gpio_write(input, gpio_read(led));
        if(gpio_read(input))
            gpio_set_on(led);
        else
            gpio_set_off(led);

        gpio_write(led2, !val);
        val = gpio_read(led2);
        delay_cycles(1000000);
    }
}
