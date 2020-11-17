#include <stdbool.h>
#include "gpio_interrupts.h"
#include "interrupts.h"
#include "assert.h"
#include "gpio.h"
#include <stdint.h> 
#include "printf.h"
/*
 * Module to configure GPIO interrupts for Raspberry Pi.
 * Because all of the GPIO pins share a small set of GPIO
 * interrupts, you need a level of indirectiom to be able
 * to handle interrupts for particular pins. This module
 * allows a client to register one handler for each
 * GPIO pin.
 *
 * Author: Philip Levis <pal@cs.stanford.edu>
 *
 * Last update:   May 2020
 */

// create array of handlers for pins 
static struct { 
	handler_fn_t fn; 
} handlers_pins[GPIO_PIN_LAST + 1]; 

struct pending_t { 
	uint32_t eds[2]; 
}; 

#define PENDING_BASE (void*) 0x20200040 // from BCM 2835 ARM peripherals manual 

static volatile struct pending_t *const pending = PENDING_BASE; 

static bool gpio_interrupts_handler(unsigned int pc); 

bool gpio_default_handler(unsigned int pc) { return false; }

/*
 * `gpio_interrupts_init`
 *
 * Initialize the GPIO interrupt modules. The init function must be 
 * called before any calls to other functions in this module.
 * The init function configures gpio interrupts to a clean state.
 * This function registers a handler for GPIO interrupts, which it
 * uses to call handlers on a per-pin basis.
 *
 * Disables GPIO interrupts if they were previously enabled.
 * 
 */
void gpio_interrupts_init(void) 
{
	// disable interrupts 
	gpio_interrupts_disable(); 
	// reset all handlers for pins to default handlers 
	for (int i = 0; i< 54; i++) { 
		handlers_pins[i].fn = gpio_default_handler;
	} 
	// set a handler for source (big boy handler) 
	interrupts_register_handler(INTERRUPTS_GPIO3, gpio_interrupts_handler); 
}

extern unsigned int count_leading_zeroes(unsigned int val);

// big boy handler 
static bool gpio_interrupts_handler(unsigned int pc) { 
//     check EDS for which pin went on 
	int pin = 31 - count_leading_zeroes(pending->eds[0]);
        //printf("%d", pin); 
	// check for the next 22 if there was no set pin in pending.eds0 
	if (pin == -1) { 
	    pin = 31 - count_leading_zeroes(pending->eds[1]) + 32; 
  	} 	  
//     if (that was a valid pin) 
	if (pin >= GPIO_PIN_FIRST && pin <= GPIO_PIN_LAST){ 
	    return handlers_pins[pin].fn(pc); 
            //return true; 
	} 
	return false; 
	//return gpio_interrupts_register_handler(pin, handlers_pin[pin].fn(pc)); 
} 

/*
 * `gpio_interrupts_enable`
 *
 * Enables GPIO interrupts.
 */
void gpio_interrupts_enable(void) 
{
	interrupts_enable_source(INTERRUPTS_GPIO3); 
}

/*
 * `gpio_interrupts_disable`
 *
 * Disables GPIO interrupts.
 */
void gpio_interrupts_disable(void) 
{
	interrupts_disable_source(INTERRUPTS_GPIO3); 
}

/* 
 * `interrupts_attach_handler`
 *
 * Attach a handler function to a given GPIO pin. Each pin
 * source can have one handler: further dispatch should be invoked by
 * the handler itself. Whether or not a particular pin will
 * generate interrupts is specified by the events system,
 * defined in `gpioextra.h`. 
 *
 * Asserts if failed to install handler (e.g., the pin is invalid).
 * Pins are defined in `gpio.h`.
 */
handler_fn_t gpio_interrupts_register_handler(unsigned int pin, handler_fn_t fn) 
{
	assert(pin <= GPIO_PIN_LAST && pin >= GPIO_PIN_FIRST); 
        handler_fn_t old_handler = handlers_pins[pin].fn; 
	handlers_pins[pin].fn = fn; 
	return old_handler; 	
	//return 0; 
}

