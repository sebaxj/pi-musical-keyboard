#include <stdint.h> 
#include "printf.h"
#include "uart.h"
#include "timer.h"
#include "audio.h"
#include "keyboard.h"
#include "interrupts.h"
#include "sin8.h"
#include "pi.h"
#include "shell.h"



#define ESC_SCANCODE 0x76
#define ENTER_SCANCODE 0x5A
#define PHASE (99 << 18)
#define TESTING_MODE 1

enum PHASE_CONSTANTS {
    PHASE_A = 0b1100000000111101001010101,
    PHASE_A_sharp = 0b1100101110101011100011111,
    PHASE_B = 0b1101011111000111111111011,
    PHASE_C = 0b1110010010011100101111101,
    PHASE_C_sharp = 0b1111001000110100110010011,
    PHASE_D = 0b10000000010011011101111111,
    PHASE_D_sharp = 0b10000111111011101111011101,
    PHASE_E = 0b10010000000001000100010010,
    PHASE_F = 0b10011000100101000111110110,
    PHASE_F_sharp = 0b10100001101001110010000011, 
    PHASE_G = 0b10101011010000111110011000,
    PHASE_G_sharp = 0b10110101011100110001001111,
};

enum musical_keys{ 
	A_code = 0x15, 
	Asharp_code = 0x1d, 
	B_code = 0x24,
	C_code = 0x2d, 
	Csharp_code = 0x2c, 
	D_code = 0x35,
	Dsharp_code = 0x3c, 
	E_code = 0x43, 
	F_code = 0x44, 
	Fsharp_code = 0x4d,
	G_code = 0x54,
	Gsharp_code = 0x5b
};
/*

static void test_audio_write_u8_scale(void) {
    // play A major scale with 1 sec intervals
    // third parameter is 8 because it passes the multiplier of 
    // the base tone length which is 125,000 us
    audio_write_u8(sinewave, PHASE_A, 8);
    timer_delay(1);
    audio_write_u8(sinewave, PHASE_B, 8);
    timer_delay(1);
    audio_write_u8(sinewave, PHASE_C_sharp, 8);
    timer_delay(1);
    audio_write_u8(sinewave, PHASE_D, 8);
    timer_delay(1);
    audio_write_u8(sinewave, PHASE_E, 8);
    timer_delay(1);
    audio_write_u8(sinewave, PHASE_F_sharp, 8);
    timer_delay(1);
    audio_write_u8(sinewave, PHASE_G_sharp, 8);
    timer_delay(1);
    audio_write_u8(sinewave, 2 * PHASE_A, 8); // 2 * base A (220 Hz) to produce octave, 440 Hz
    timer_delay(1);
}
*/

static key_action_t play_note(unsigned phase, key_action_t action){ 
	while (action.what == KEY_PRESS) { 
		audio_write_u8(sinewave, phase, 1); 
		action = keyboard_read_sequence(); 
	} 
	return action; 
} 

static void test_audio_write_u8_key(void) {
    while (1) { 
	    key_action_t action = keyboard_read_sequence();
	    if (action.keycode == ESC_SCANCODE) break;
            //printf("%s [%02x]\n", action.what == KEY_PRESS ? "Press" : "Release", action.keycode);
	    switch(action.keycode) { 
		    case A_code:
		    		action = play_note(PHASE_A, action); 
				break; 
		    case Asharp_code:
				action = play_note(PHASE_A_sharp, action); 
			    	break; 
		    case B_code: 
				action = play_note(PHASE_B, action); 
			    	break; 
		    case C_code: 
				action = play_note(PHASE_C, action); 
			    	break;
		    case Csharp_code: 
				action = play_note(PHASE_C_sharp, action): 
				break; 
		    case D_code: 
				action = play_note(PHASE_D, action); 
			    	break; 
		    case Dsharp_code: 
				action = play_note(PHASE_D_sharp, action); 
			    	break; 
		    case E_code: 
				action = play_note(PHASE_E, action); 
			    	break;
		    case F_code: 
				action = play_note(PHASE_F, action); 
			    	break; 
		    case Fsharp_code: 
				action = play_note(PHASE_F_sharp, action); 
			    	break; 
		    case G_code: 
				action = play_note(PHASE_G, action); 
			    	break; 
		    case Gsharp_code: 
				action = play_note(PHASE_G_sharp, action): 
			    	break; 
	    } 	
    }
}



void main(void) {
    uart_init();
    audio_init();
    interrupts_init();
    gpio_init(); // needed for Isma's keyboard to work 
    keyboard_init(KEYBOARD_CLOCK, KEYBOARD_DATA);
    interrupts_global_enable(); // needed for Isma's keyboard module to work 

    /* AUDIO MODULE */
    /*
    printf("*** AUDIO MODULE ***\n\n");
    printf("Testing single: A Major Scale\n\n");
    test_audio_write_u8_scale();
    */

    /* KEYBOARD MODULE */
    printf("Testing Key Press: Press ESC to quit\n\n");
    test_audio_write_u8_key();

    #if 0
    /* SHELL MODULE */
    printf("\nTesting Shell Module with 'printf\n\n'");
    shell_init(printf); 
    shell_run();

    /* GRAPHICS MODULE */
    printf("Testing Graphics Display: C Major Scale");
    #endif

    printf("All done!\n");
    printf("Pi rebooting...\nSee ya back at the bootloader!");
    uart_putchar(EOT);
    pi_reboot();
}

