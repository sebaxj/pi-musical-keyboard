#include <stdbool.h>
#include "gpio_interrupts.h"
#include "assert.h"

// This struct is declared to match memory layout of the interrupt registers
struct interrupt_t {
    unsigned int reservedA;
    unsigned int pending[2];
    unsigned int fiq_control;
    unsigned int enable[2];
    unsigned int reservedB;
    unsigned int disable[2];
    unsigned int reservedC;
};

static struct {
    handler_fn_t fn;
} handlers[INTERRUPTS_COUNT];

#define INTERRUPT_CONTROLLER_BASE (void *)0x2000B200
static volatile struct interrupt_t * const interrupt = INTERRUPT_CONTROLLER_BASE;

// Returns whether a given IRQ is safe to be used. The pin must be a valid gpio pin recieving input.
static bool is_safe(unsigned int irq) 
{
    if((irq >= GPIO_PIN0 && irq <= GPIO_PIN_LAST) && 
    gpio_get_function(irq) == GPIO_FUNC_INPUT) {return true;}
    return false;
}

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
    // disable all sources
    interrupt->disable[0] = 0xffffffff;
    interrupt->disable[1] = 0xffffffff;
}

/*
 * `gpio_interrupts_enable`
 *
 * Enables GPIO interrupts.
 */
void gpio_interrupts_enable(void) 
{
    unsigned int shift = INTERRUPTS_GPIO3 - INTERRUPTS_BASIC_BASE;
    interrupt->enable[0] |= 1 << shift;
}

/*
 * `gpio_interrupts_disable`
 *
 * Disables GPIO interrupts.
 */
void gpio_interrupts_disable(void) 
{
    interrupt->disable[0] = 0xffffffff;
    interrupt->disable[1] = 0xffffffff;
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
    assert(is_safe(pin));
    
    handler_fn_t old_handler = handlers[pin].fn;
    handlers[pin].fn = fn;
    return old_handler;
}