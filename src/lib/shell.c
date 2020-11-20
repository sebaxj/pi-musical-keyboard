#include "shell.h"
#include "shell_commands.h"
#include "uart.h"
#include "keyboard.h"
#include "strings.h"
#include "malloc.h"
#include "pi.h"
#include "timer.h"
#include "console.h"
#include "audio.h"
#include <stdint.h> 
#include "printf.h"
#include "gl.h"
#include "piano.h"

/* Include Instrument Waveform header files */
#include "AKWF_ebass.h"
#include "AKWF_eguitar.h"
#include "AKWF_eorgan.h"
#include "AKWF_hvoice.h"
#include "AKWF_piano.h"
#include "AKWF_violin.h"

/**********************
 * shell.c module
 * 
 * Desc
 * 
 * ********************/

/*
 * Constants, Globals, and Statics
 */
#define LINE_LEN 160
#define ENTER_SCANCODE 0x5A
#define ESC_SCANCODE 0x76

static int16_t *inst_wav[];
static color_t color_press = GL_MAGENTA; 

/*
 * C compilation macros
 */
#define min(x, y) (x < y ? x : y)

/* 
 * enum to store the binary phase representation of
 * note frequencies
 * 
 * '0b' represents binary number.
 * For notes A through C#, the first digit represents the 8 MSB
 * For notes D thorugh G#, the first two digits represent the 8 MSB
 */
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
    PHASE_A_upper = 0b11000000001111010010101010,
};

/* i
 * enum to map keyboard scancode to music note
 * 
 * '0x' represents hexadecimal number.
 * Note A is represented by the scancode 0x15 (Q),
 * Note G# is representedd by the scancode 0x5B (]})
 */
enum musical_keys { 
	A_code = 0x0d, // tab
	Asharp_code = 0x15, // Q
	B_code = 0x1d,
	C_code = 0x24, 
	Csharp_code = 0x2d, 
	D_code = 0x2c,
	Dsharp_code = 0x35, 
	E_code = 0x3c, 
	F_code = 0x43, 
	Fsharp_code = 0x44,
	G_code = 0x4d,
	Gsharp_code = 0x54,
	Aupper_code = 0x5b,  
    CHANGE_INTS = 0x1A, // Z 
};

static formatted_fn_t shell_printf;
static int cmd_music(int argc, const char *argv[]); 

static char *strndup(const char *src, size_t n) {
    n = min(strlen(src), n);
    char *dst = malloc(n + 1);
   
    memcpy(dst, src, n);
    dst[n] = '\0';
    return dst;
}

static int isspace(char ch) {
    if(ch == ' ' || ch == '\t' || ch == '\n') {
        return 1;
    }
    return 0;
}

static int tokenize(const char *line, char *array[], int max) {
    int ntokens = 0;
    const char *cur = line;

    while(ntokens < max) {
        while(isspace(*cur)) cur++; // skip spaces
        if(*cur == '\0') break; // no more non-space
        const char *start = cur;
        while(*cur != '\0' && !isspace(*cur)) cur++; // advance to end
        char *result = strndup(start, cur - start);
        array[ntokens++] = result;
    }
    return ntokens;
}

static const command_t commands[] = {
    {"help",   "<cmd> prints a list of commands or description of cmd", cmd_help},
    {"echo",   "<...> echos the user input to the screen", cmd_echo},
    {"reboot", "reboot the Raspberry Pi back to the bootloader", cmd_reboot},
    {"peek", "This command takes one argument: [address]. It prints the 4-byte value stored at memory address [address]", cmd_peek},
    {"poke", "This command takes two arguments: [address] [value]. The poke function stores value into the memory at [address]", cmd_poke},
    {"music", "This command turns the keyboard into a musical keyboard", cmd_music} 
};

static int evaluate_instrument(const char *line) {
    int result = -1;

    // parse argument
    int cmd_result = 1;
    for(int i = 0; i < sizeof(instruments); i++) {
        if(strcmp(line, instruments[i]) == 0) {
            switch (i) {
            case 1: // electric bass
                *inst_wav = AKWF_ebass_0001;
                break;
            case 2: // electric guitar
                *inst_wav = AKWF_eguitar_0001;
                break;
            case 3: // organ
                *inst_wav = AKWF_eorgan_0001;
                break;
            case 4: // vocals
                *inst_wav = AKWF_hvoice_0001;
                break;
            case 5: // piano
                *inst_wav = AKWF_piano_0001;
                break;
            case 6: // violin
                *inst_wav = AKWF_violin_0001;
                break;
            }
        }
    }
    if(cmd_result) {
        shell_printf("error: no such instrument '%s'\n", line);
    }

    return result;
}

static key_action_t play_note(unsigned phase, key_action_t action){ 
	timer_delay_ms(400); // almost resolves the cutting issue at the beginning, if we delay more it's even better but it makes the keyboard asynchronous with the sound. 
	while (action.what == KEY_PRESS) { 
        	audio_write_i16(*inst_wav, phase, 1); 
		action = keyboard_read_sequence(); 
	} 
	return action; 
} 

static int cmd_music(int argc, const char *argv[]) { 
    draw_piano();
    *inst_wav = AKWF_piano_0001; // initialized with piano default
    while (1) { 
	    key_action_t action = keyboard_read_sequence();
	    if (action.keycode == ESC_SCANCODE) break;
	    switch(action.keycode) { 
		    case A_code: 
			    draw_middle_key(piano_keys[key_A], color_press, MIDDLE_KEY); 
			    action = play_note(PHASE_A, action); 
			    draw_middle_key(piano_keys[key_A], GL_WHITE, MIDDLE_KEY);
			    break; 
		    case Asharp_code:
			    draw_sharp(piano_keys[key_A], color_press);
			    action = play_note(PHASE_A_sharp, action); 
			    draw_sharp(piano_keys[key_A], GL_BLACK); 
			    break; 
		    case B_code: 
			    draw_middle_key(piano_keys[key_B], color_press, LEFT_KEY); 
			    action = play_note(PHASE_B, action); 
			    draw_middle_key(piano_keys[key_B], GL_WHITE, LEFT_KEY); 
			    break; 
		    case C_code: 
			    draw_middle_key(piano_keys[key_C], color_press, RIGHT_KEY); 
			    action = play_note(PHASE_C, action);
			    draw_middle_key(piano_keys[key_C], GL_WHITE, RIGHT_KEY); 
			    break;
		    case Csharp_code: 
			    draw_sharp(piano_keys[key_C], color_press); 
			    action = play_note(PHASE_C_sharp, action); 
			    draw_sharp(piano_keys[key_C], GL_BLACK); 
			    break; 
		    case D_code: 
			    draw_middle_key(piano_keys[key_D], color_press, MIDDLE_KEY); 
			    action = play_note(PHASE_D, action); 
			    draw_middle_key(piano_keys[key_D], GL_WHITE, MIDDLE_KEY);
			    break; 
		    case Dsharp_code: 
			    draw_sharp(piano_keys[key_D], color_press); 
			    action = play_note(PHASE_D_sharp, action); 
			    draw_sharp(piano_keys[key_D], GL_BLACK); 
			    break; 
		    case E_code: 
			    draw_middle_key(piano_keys[key_E], color_press, LEFT_KEY); 
			    action = play_note(PHASE_E, action); 
			    draw_middle_key(piano_keys[key_E], GL_WHITE, LEFT_KEY); 
			    break;
		    case F_code: 
			    draw_middle_key(piano_keys[key_F], color_press, RIGHT_KEY); 
			    action = play_note(PHASE_F, action); 
			    draw_middle_key(piano_keys[key_F], GL_WHITE, RIGHT_KEY);
			    break; 
		    case Fsharp_code: 
			    draw_sharp(piano_keys[key_F], color_press);
			    action = play_note(PHASE_F_sharp, action); 
			    draw_sharp(piano_keys[key_F], GL_BLACK); 
			    break; 
		    case G_code:
			    draw_middle_key(piano_keys[key_G], color_press, MIDDLE_KEY); 
			    action = play_note(PHASE_G, action); 
			    draw_middle_key(piano_keys[key_G], GL_WHITE, MIDDLE_KEY); 
			    break; 
		    case Gsharp_code: 
			    draw_sharp(piano_keys[key_G], color_press); 
			    action = play_note(PHASE_G_sharp, action); 
			    draw_sharp(piano_keys[key_G], GL_BLACK); 
			    break;
		    // case Aupper_code: 
			//     draw_middle_key(piano_keys[key_A_upper], color_press, MIDDLE_KEY); 
			//     action = play_note(PHASE_A_upper, action); 
			//     draw_middle_key(piano_keys[key_A_upper], GL_WHITE, MIDDLE_KEY); 
			//     break; 
            case CHANGE_INTS:
                console_clear();
                char line[LINE_LEN];
                shell_printf("Instrument Sound Library:\nPress 'Q' to exit selection.\n");

                for(int i = 0; i < (sizeof(instruments) / sizeof(*instruments)); i++) {
                    shell_printf("> %s\n", instruments[i]);
                }

                shell_printf("Please type instrument> ");
                shell_readline(line, sizeof(line));
                if(line[0] == 'Q' || line[0] == 'q') {
                    draw_piano();
                    break;
                }

                evaluate_instrument(line);
                break; 
	    } 	    
    } 
    console_clear();
    shell_printf("Welcome to the CS107E shell. Remember to type on your PS/2 keyboard!\n");
    return 0; 
} 

int cmd_help(int argc, const char *argv[]) 
{
    for(int i = 0; i < (sizeof(commands) / sizeof(*commands)); i++) {
        shell_printf("%s: %s\n", commands[i].name, commands[i].description);
    }
    return 0;
}

int cmd_echo(int argc, const char *argv[]) 
{
    for (int i = 1; i < argc; ++i) {
        shell_printf("%s ", argv[i]);
    }
    shell_printf("\n");
    return 0;
}

int cmd_reboot(int argc, const char* argv[]) {
    shell_printf("Pi rebooting...\nSee ya back at the bootloader!");
    timer_delay(1);
    uart_putchar(EOT);
    pi_reboot();
    return 0;
}

int cmd_peek(int argc, const char* argv[]) {
    if(argc != 2) { // if no address
        shell_printf("error: peek expects 1 argument [address]\n");
    }
    unsigned int hex_addr = strtonum((const char *)argv[1], NULL);
    if(hex_addr == 0 && (argv[1])[0] != '0') { // if bad address
        shell_printf("error: peek cannot convert '%s'\n", argv[1]);
        return -1;
    } else if(hex_addr % 4 != 0) { // if unaligned address
        shell_printf("error: peek address must be 4-byte aligned\n");
        return -1;
    }

    // cast int address to pointer type
    unsigned int *p = (unsigned int *)hex_addr;
    shell_printf("0x%x: %d\n", hex_addr, *p);
    return 0;
}

int cmd_poke(int argc, const char* argv[]) {
    if(argc != 3) { // no value
        shell_printf("error: poke expects 2 arguments [address] [value]\n");
        return -1;
    } 

    unsigned int hex_addr = strtonum((const char *)argv[1], NULL); // addr
    unsigned int value = strtonum((const char *)argv[2], NULL); // value
    
    if(hex_addr == 0) { // if bad address
        shell_printf("error: peek cannot convert '%s'\n", argv[1]);
        return -1;
    } else if(hex_addr % 4 != 0) { // if unaligned address
        shell_printf("error: peek address must be 4-byte aligned\n");
        return -1;
    } else if(value == 0 && (argv[2])[0] != '0') { // bad value
        shell_printf("error: peek cannot convert '%s'\n", argv[2]);
        return -1;
    }

    // cast int address to pointer type
    unsigned int *p = (unsigned int *)hex_addr;
    *p = value;
    return 0;
}

void shell_init(formatted_fn_t print_fn)
{
    shell_printf = print_fn;
}

void shell_bell(void)
{
    uart_putchar('\a');
}

void shell_readline(char buf[], size_t bufsize)
{
    int num = 0;
    
    wait_for_char:
    while(1) {
        char c = keyboard_read_next();
        if (c == ps2_keys[ENTER_SCANCODE].ch) {
            if(num == 0) {
                shell_printf("\nPi> ");
                goto wait_for_char;
            }
            shell_printf("\n");
            buf[num] = '\0';
            break;
        }
        if(num >= bufsize || (num == 0 && c == '\b')) {
            shell_bell();
        } else if(c == '\b') { // backspace
            if(num != 0) {
                shell_printf("%c%c%c", '\b', ' ', '\b');
                num--;
            } else {
                shell_bell();
            }
        } else if (c >= '\t' && c <= 0x80) {
            buf[num] = c;
            shell_printf("%c", buf[num]);
            num++;
        }
    }
}

int shell_evaluate(const char *line)
{
    int max = strlen(line);
    char *tokens[max];
    int result = -1;
    int num = tokenize(line, tokens, max);
    char *arg = tokens[0];

    // parse argument
    int cmd_result = 1;
    for(int i = 0; i < sizeof(commands); i++) {
        if(strcmp(arg, commands[i].name) == 0) {
            int (*funct_ptr)(int, const char *[]);
            funct_ptr = commands[i].fn;
            result = (*funct_ptr)(num, (const char **)tokens);
            cmd_result = 0;
        }
    }
    if(cmd_result) {
        shell_printf("error: no such command '%s'\n", arg);
    }

    // free heap-allocated memory
    for(int i = 0; i < num; i++) {
        free(tokens[i]);
    }
    return result;
}

void shell_run(void)
{
    shell_printf("Welcome to the CS107E shell. Remember to type on your PS/2 keyboard!\n");
    
    while (1) 
    {
        char line[LINE_LEN];

        shell_printf("Pi> ");
        shell_readline(line, sizeof(line));
        shell_evaluate(line);
    }
}
