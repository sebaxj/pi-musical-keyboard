#include "gpio.h"

/*
Constants
*/

#define GPIO_FSEL0 = ((unsigned int*) 0x20200000)
#define SET (int) 1

/*
Struct to match memory layout of gpio registers
*/

struct gpio {
    unsigned int fsel[6];   // function select
    unsigned int reservedA;
    unsigned int set[2];    // set
    unsigned int reservedB;
    unsigned int clr[2];    // clear
    unsigned int reservedC;
    unsigned int lev[2];    // level
};

volatile struct gpio* gpio_map = (struct gpio*)0x20200000;

void gpio_init(void) {
}

void gpio_set_function(unsigned int pin, unsigned int function) {
    if(pin > GPIO_PIN_LAST || function > GPIO_FUNC_ALT3) {return;} // error if pin out of bounds else {
    
    //Take pin number to be set, 0 register, make bit mask, OR bit to set register as function
    unsigned int bit = (3 * (pin % 10)); // Create bit mask
    unsigned int curr = gpio_map->fsel[pin / 10]; // Read current register
    curr &= ~(7 << bit); // 0 GPIO mode bits for that port
    curr |= (function << bit); // Logical OR GPIO mode bits into desired register
    gpio_map->fsel[pin / 10] = curr; // Write value to register
}

unsigned int gpio_get_function(unsigned int pin) {
    if(pin > GPIO_PIN_LAST) {return GPIO_INVALID_REQUEST;} // error if pin out of bounds

    // desired range of bits starts at 3 * (pin % 10)
    int num_bits_set = 0;
    for(int i = 0; i < 3; i++) { // read each of 3 bits
        if((gpio_map->fsel[pin / 10] >> (i + 3 * (pin % 10))) & SET) { // if bit set, modify num_bit_set
            num_bits_set |= SET << i;
        }
    }
    return num_bits_set;
}

void gpio_set_input(unsigned int pin) {
    gpio_set_function(pin, GPIO_FUNC_INPUT);
}

void gpio_set_output(unsigned int pin) {
    gpio_set_function(pin, GPIO_FUNC_OUTPUT);
}

void gpio_write(unsigned int pin, unsigned int value) {
    if(pin > GPIO_PIN_LAST || value > SET) {return;}
    if(value) {
        gpio_map->set[pin / 32] = (SET << (pin % 32));
    }
    else {
        gpio_map->clr[pin / 32] = (SET << (pin % 32));
    }
}

unsigned int gpio_read(unsigned int pin) {
    if (pin > GPIO_PIN_LAST) {return GPIO_INVALID_REQUEST;}
    unsigned int level = (SET << (pin % 32)) & gpio_map->lev[pin / 32];
    return level >> (pin % 32);
}
