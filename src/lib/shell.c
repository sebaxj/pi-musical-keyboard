#include "shell.h"
#include "shell_commands.h"
#include "uart.h"
#include "keyboard.h"
#include "strings.h"
#include "malloc.h"
#include "pi.h"
#include "timer.h"
#include "console.h"


#define LINE_LEN 160
#define ENTER_SCANCODE 0x5A
#define min(x, y) (x < y ? x : y)
#define ESC_SCANCODE 0x76

#define TESTING_MODE 0 // change to 1 to enable shell.c to run through test array when initializing



static formatted_fn_t shell_printf;

int cmd_music(int argc, const char *argv[]); 

#if TESTING_MODE
static const char *tests[9] = {"echo Hello, world!", "", " ", "peek bob", "help", "peek 0xFFFC", "poke 0xFFFC 1", "peek 0xFFFC", "poke 0xF"};
#endif

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

int cmd_music(int argc, const char *argv[]){ 
    draw_piano(); // from console.c
    while (1) { 
	    key_action_t action = keyboard_read_sequence(); 
	    if (action.keycode == ESC_SCANCODE) break; 
    } 
    shell_printf("Welcome to the CS107E shell. Remember to type on your PS/2 keyboard!\n");
    return 0; 
} 
 

int cmd_help(int argc, const char *argv[]) 
{
    for(int i = 0; i < 5; i++) {
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

    #if TESTING_MODE
    shell_printf("Testing...\n");
    for(int i = 0; i < sizeof(tests); i++) {
        shell_evaluate(tests[i]);
    }
    #endif
    
    while (1) 
    {
        char line[LINE_LEN];

        shell_printf("Pi> ");
        shell_readline(line, sizeof(line));
        shell_evaluate(line);
    }
}
