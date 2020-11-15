#include "gpio.h"
#include "gpioextra.h"
#include "keyboard.h"
#include "ps2.h"
#include "timer.h"

#define NUM_DATA_BITS 8
static unsigned int CLK, DATA;
static unsigned int mod = 0;

// enum to store mod keycodes
enum MOD_KEYCODES {
    LSHIFT = 0x12,
    RSHIFT = 0x59,
    ALT = 0x11,
    CTRL = 0x14,
    CAPS_LOCK = 0x58,
    SCROLL_LOCK = 0x7E,
    NUM_LOCK = 0x77,
};

static int is_not_letter(unsigned char key) {
    if((key == 0x15) || 
    (key >= 0x1A && key <= 0x1D) || 
    (key >= 0x21 && key <= 0x24) || 
    (key >= 0x2A && key <= 0x2D) || 
    (key >= 0x31 && key <= 0x35) || 
    (key >= 0x3A && key <= 0x3C) || 
    (key >= 0x42 && key <= 0x44) || 
    (key == 0x4B) || 
    (key == 0x4D)) {
        return 0;
    }
    return 1;
}

void keyboard_init(unsigned int clock_gpio, unsigned int data_gpio) 
{
    CLK = clock_gpio;
    gpio_set_input(CLK);
    gpio_set_pullup(CLK);
 
    DATA = data_gpio;
    gpio_set_input(DATA);
    gpio_set_pullup(DATA);
}

static int read_bit(void) {
    // wait for clock falling edge
    while(gpio_read(CLK) == 0);
    while(gpio_read(CLK) == 1);

    // read data
    return gpio_read(DATA);
}

unsigned char keyboard_read_scancode(void) 
{
    int parity = 0, value = 0, num_bits = 0, current_clock_edge;
    while(!parity) {
        read_start_bit: if(!read_bit()) {
            current_clock_edge = timer_get_ticks(); // time of falling edge
            for(int i = 0; i < 8; i++) {
                int bit = read_bit();

                // check time between falling edges, if greater than 3 ms, reset state, assume falling edge
                if(timer_get_ticks() - current_clock_edge > 3000) {
                    // timeout reset
                    parity = 0; value = 0; num_bits = 0; 
                    goto read_start_bit;
                } else {
                    current_clock_edge = timer_get_ticks();
                
                    // compute incoming bit
                    num_bits += bit;
                    int mask = 1 << i;
                    value = ((value & ~mask) | ((bit << i) & mask));
                }
            }
            num_bits += read_bit(); // read parity bit
            if(num_bits % 2 && read_bit()) {
                parity = 1; // odd parity
            } else {
                value = 0;
                num_bits = 0;
            }
        }
    }
    return value;
}

key_action_t keyboard_read_sequence(void)
{
    key_action_t action;
    while(1) {
        unsigned char scancode = keyboard_read_scancode();
        switch(scancode) {
            case PS2_CODE_EXTENDED: // extended key
                scancode = keyboard_read_scancode();
                switch(scancode) {
                    case PS2_CODE_RELEASE: // key release
                        action.keycode = keyboard_read_scancode();
                        action.what = KEY_RELEASE;
                        return action;
                        break;
                    default: // key press
                        action.keycode = keyboard_read_scancode();
                        action.what = KEY_PRESS;
                        return action;
                }
                break;
            case PS2_CODE_RELEASE: // ordinary key release
                action.keycode = keyboard_read_scancode();
                action.what = KEY_RELEASE;
                return action;
                break;
            default: // ordinary key press
                action.keycode = scancode;
                action.what = KEY_PRESS;
                return action;
        }
    }
}

key_event_t keyboard_read_event(void) 
{
    key_event_t event;
    key_action_t action = keyboard_read_sequence();
    event.action = action;
    event.key = ps2_keys[action.keycode];
    int mask = 0;

    // set modifier state using bit masking on static mod
    switch(action.keycode) {
        case LSHIFT:
            mask = 1 << 0;
            if(action.what == KEY_PRESS) {
                mod = ((mod & ~mask) | (KEYBOARD_MOD_SHIFT & mask));
            } else {
                mod = ((mod & ~mask) | (0 << 0 & mask));
            }
            break;
        case RSHIFT:
            mask = 1 << 0;
            if(action.what == KEY_PRESS) {
                mod = ((mod & ~mask) | (KEYBOARD_MOD_SHIFT & mask));
            } else {
                mod = ((mod & ~mask) | (0 << 0 & mask));
            }
            break;
        case ALT:
            mask = 1 << 1;
            if(action.what == KEY_PRESS) {
                mod = ((mod & ~mask) | (KEYBOARD_MOD_ALT & mask));
            } else {
                mod = ((mod & ~mask) | (0 << 1 & mask));
            }
            break;
        case CTRL:
            mask = 1 << 2;
            if(action.what == KEY_PRESS) {
                mod = ((mod & ~mask) | (KEYBOARD_MOD_CTRL & mask));
            } else {
                mod = ((mod & ~mask) | (0 << 2 & mask));
            }
            break;
        case CAPS_LOCK:
            mask = 1 << 3;
            if(action.what == KEY_PRESS) {
                if(mod & (1 << 3)) { // lock is on
                    mod = ((mod & ~mask) | (0 << 3 & mask));
                } else {
                    mod = ((mod & ~mask) | (KEYBOARD_MOD_CAPS_LOCK & mask));
                } 
            }
            break;
        case SCROLL_LOCK:
            mask = 1 << 4;
            if(action.what == KEY_PRESS) {
                if(mod & (1 << 4)) { // lock is on
                    mod = ((mod & ~mask) | (0 << 4 & mask));
                } else {
                    mod = ((mod & ~mask) | (KEYBOARD_MOD_SCROLL_LOCK & mask));
                } 
            }
            break;
        case NUM_LOCK:
            mask = 1 << 5;
            if(action.what == KEY_PRESS) {
                if(mod & (1 << 5)) { // lock is on
                    mod = ((mod & ~mask) | (0 << 5 & mask));
                } else {
                    mod = ((mod & ~mask) | (KEYBOARD_MOD_NUM_LOCK & mask));
                } 
            }
            break;
        
    }

    event.modifiers = mod;
    return event;
}

unsigned char keyboard_read_next(void) 
{
    while(1) {
        key_event_t event = keyboard_read_event();
        if(event.action.what == KEY_PRESS // check that event is a key press and a non modifier key
        && event.action.keycode != LSHIFT 
        && event.action.keycode != RSHIFT
        && event.action.keycode != ALT
        && event.action.keycode != CTRL
        && event.action.keycode != CAPS_LOCK
        && event.action.keycode != SCROLL_LOCK
        && event.action.keycode != NUM_LOCK) {
            if((mod & (1 << 0)) || (mod & (1 << 3))) { // if caps lock or shift
                if(event.key.other_ch) { // if key has an other_ch

                    // if caps lock is activated without shift, num keys return event.key.ch
                    if((mod & (1 << 3)) && !(mod & (1 << 0)) && is_not_letter(event.action.keycode)) {
                        return event.key.ch;
                    }
                    return event.key.other_ch;
                } else {
                   return event.key.ch; 
                }
            }
            return event.key.ch;
        }
    }
}
