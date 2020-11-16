#include "gpio.h"
#include "printf.h"
#include "uart.h"
#include "keyboard.h"
#include "timer.h"
#include "ps2.h"
#include "gpioextra.h"
#include "gpio_interrupts.h"
#include <stdbool.h>
#include "ringbuffer.h"

static unsigned CLK = GPIO_PIN3;
static unsigned DATA = GPIO_PIN4;

static unsigned int MODIFIERS = 0; 

#define RESET 1

enum { 
	START_BIT = 0,
	END_BIT = 1,
}; 

enum { PS2_CMD_RESET = 0xFF,
	PS2_CODE_ACK = 0xFA,
	PS2_CMD_FLAGS = 0xED ,
	PS2_CMD_ENABLE_DATA_REPORTING = 0xF4,
};

#if RESET 
static int has_odd_parity(unsigned char code){
	unsigned int sum = 0;
	for (unsigned int i =0; i<8; i++){
		sum += code & 1;
		code >>= 1;
	}
	return !(sum % 2);
}


static void write_bit(int nbit, unsigned char code)
{
    switch (nbit) {
        case 1:
            gpio_write(DATA, START_BIT);
            break;
        case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9:
            gpio_write(DATA, (code >> (nbit-2)) & 1);
            break;
        case 10:
            gpio_write(DATA, has_odd_parity(code));
            break;
        case 11:
            gpio_write(DATA, END_BIT);
            break;
    }
}


void wait_for_falling_clock_edge(){
	while (gpio_read(CLK) == 0);
	while (gpio_read(CLK) == 1);
}

static void ps2_write(unsigned char command)
{
    gpio_set_output(CLK);   // swap GPIO from read to write
    gpio_set_output(DATA);
    gpio_write(CLK, 0);     // bring clock line low to inhibit communication
    timer_delay_us(200);    // hold time > 100 us
    gpio_write(DATA, 0);    // initiate request-to-send, this is START bit
    gpio_write(CLK, 1);
    gpio_set_input(CLK);    // release clock line, device will now pulse clock for host to write

    for (int i = 2; i <= 11; i++) {  // START bit sent above, now send bits 2-11
        wait_for_falling_clock_edge();
        write_bit(i, command);
    }

    gpio_set_input(DATA);   // done writing, exit from request-to-send
    wait_for_falling_clock_edge();
}
#endif 

static unsigned char scancode = 0; 
static int count = 0; 
static int parity = 0; 

static rb_t *rb = NULL; 

static void reset_variables(void){
	scancode = 0; 
	count = 0; 
	parity = 0; 
} 


static bool clock_handler(unsigned int pc){ 
	if (gpio_check_and_clear_event(CLK)) { 
		int bit = gpio_read(DATA); 
		count = count%11; 
		switch (count) {
			case 0: 
				if (bit == START_BIT) { 
					count++; 
				} 
				break; 
			case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8: 
				scancode = scancode | (bit << (count - 1)); 
				if (bit == 1){ 
					parity++;
				} 
				count++; 
				break;
			case 9: 
				if (bit == !(parity%2)){
					count++; 
				} else { 
					reset_variables(); 
				} 
				break; 
			case 10: 
				if (bit != END_BIT) { 
					reset_variables(); 
				} else {
					count++; 
					// enqueue scancode in ringbuffer
					rb_enqueue(rb, scancode);
					reset_variables(); 
				}
				break; 
		}		
		return true; 
	} else return false; 
} 


void keyboard_init(unsigned int clock_gpio, unsigned int data_gpio) 
{
    CLK = clock_gpio;
    gpio_set_input(CLK);
    gpio_set_pullup(CLK);
 
    DATA = data_gpio;
    gpio_set_input(DATA);
    gpio_set_pullup(DATA);

    gpio_init();
    uart_init();
    // enabling GPIO interrupts 
    
    gpio_interrupts_init(); 
    gpio_enable_event_detection(CLK, GPIO_DETECT_FALLING_EDGE); 
    gpio_interrupts_register_handler(CLK, clock_handler); 
    gpio_interrupts_enable(); 

    rb = rb_new(); 
    
#if RESET 
    printf("Sending reset code.\n");
    ps2_write(PS2_CMD_RESET);
#endif 
   
}

// code needed for keyboard_read_scancode() without interrupts
/*
static int read_bit(void)
{
    wait_for_falling_clock_edge();
    return gpio_read(DATA);
}
*/

unsigned char keyboard_read_scancode(void) // still needs timer check-up
{
	// code without interrupts 
/*		
    while(1){
    	unsigned char scancode = 0;
    // read start bit
    	if (read_bit() == START_BIT){
                // read 8 bits (LSB first)
    		for (int i = 0; i <8; i++){
	    		scancode = scancode | (read_bit() << i);
    		}
                // checks if both parity bit and end bit are correct
		if (read_bit() == has_odd_parity(scancode)){
    			if (read_bit() == END_BIT) {
				return scancode;
			}
		}
	}
    }
*/     
	// code with interrupts 
	while (rb_empty(rb)) {} 
	int *scancode = NULL; 
	rb_dequeue(rb, scancode); 
	return *scancode; 
	
}

key_action_t keyboard_read_sequence(void)
{ 
    key_action_t action;
    unsigned char first = keyboard_read_scancode();
    
    if (first == 0xf0){ // ordinary release 
	    unsigned char second = keyboard_read_scancode(); 
	    action.what = KEY_RELEASE;
	    action.keycode = second; 
    } else {
	    if (first == 0xe0) {
		    unsigned char second = keyboard_read_scancode();
		    if (second == 0xf0) { // extended release 
			    action.what = KEY_RELEASE;
			    unsigned char third = keyboard_read_scancode(); 
			    action.keycode = third; 
		    } else { // extended key press
			    action.what = KEY_PRESS; 
			    action.keycode = second; 
		    } 
	    } else { // ordinary key press
		    action.what = KEY_PRESS;
		    action.keycode = first;
	    } 
    } 
    
    return action;
}

static void non_sticky(key_event_t event, keyboard_modifiers_t mod){ 
	if (event.action.what == KEY_PRESS){ 
		MODIFIERS = MODIFIERS | mod; 
	} else { 
		MODIFIERS = MODIFIERS & (~mod); 
	} 
} 

static void sticky(key_event_t event, keyboard_modifiers_t mod){ 
	if (event.action.what == KEY_PRESS) { 
		if (MODIFIERS & mod) { 
			MODIFIERS = MODIFIERS & (~mod); 
		} else { 
			MODIFIERS = MODIFIERS | mod; 
		} 
	} 
} 

key_event_t keyboard_read_event(void) 
{
    // declare a key event 
    key_event_t event;
    //event.action should be keyboard_read_sequence
    key_action_t action = keyboard_read_sequence(); 
    event.action = action;
    //event.key should reference ps2_keys array 
    event.key = ps2_keys[action.keycode]; 
    //figure out if it was one of six modifiers, if so, change MODIFIERS accordingly  
    switch (event.key.ch) {
	    case PS2_KEY_SHIFT: 
		    non_sticky(event, KEYBOARD_MOD_SHIFT);
 		    break; 
	    case PS2_KEY_ALT:
		    non_sticky(event, KEYBOARD_MOD_ALT); 
		    break;
	    case PS2_KEY_CTRL:
		    non_sticky(event, KEYBOARD_MOD_CTRL); 
		    break;
	    case PS2_KEY_CAPS_LOCK: 
		    sticky(event, KEYBOARD_MOD_CAPS_LOCK); 
		    break; 
	    case PS2_KEY_NUM_LOCK:
		    sticky(event, KEYBOARD_MOD_NUM_LOCK); 
		    break; 
	    case PS2_KEY_SCROLL_LOCK:
		    sticky(event, KEYBOARD_MOD_SCROLL_LOCK); 
		    break; 
    } 
    event.modifiers = MODIFIERS; 

    return event;
}


unsigned char keyboard_read_next(void)
{
#if RESET
    //keyboard_read_event(); // skipping the reset scancode : messes up characters typed really fast DON'T UNCOMMENT IT 
#endif 

    key_event_t event = keyboard_read_event(); 
    
    unsigned char result = 0; 

    while (event.key.ch == PS2_KEY_SHIFT || event.key.ch == PS2_KEY_ALT || event.key.ch == PS2_KEY_CAPS_LOCK || event.key.ch == PS2_KEY_CTRL || event.action.what == KEY_RELEASE) { 
	   event = keyboard_read_event(); 
    } 

    if (event.key.ch >= 0x90){ // special keys 
	   result = event.key.ch;
    } else if (event.modifiers & KEYBOARD_MOD_SHIFT) { // if shift is active
	    if (event.key.other_ch != 0) { 
	   	result = event.key.other_ch; 
	    } else { 
		    result = event.key.ch; 
	    } 
    } else if (event.modifiers & KEYBOARD_MOD_CAPS_LOCK){ // if shift is not active but caps is active 
	   if (event.key.ch >= 'a' && event.key.ch <= 'z') { // caps only has effect on letters 
		  result = event.key.other_ch; 
	   } else {
		  result = event.key.ch; 
	   } 
    } else {
	   result = event.key.ch; 
    } 
    return result;
}
