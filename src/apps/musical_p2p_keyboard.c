#include <stdint.h> 
#include "printf.h"
#include "uart.h"
#include "filters.h"
#include "timer.h"
#include "audio.h"
//#include "sin8.h"
#include "gpio.h"
#include "shell.h"
#include "console.h"
#include "keyboard.h"
#include "interrupts.h"

#define PHASE (99<<18)

#define NUM_ROWS 20 
#define NUM_COLS 40 

void main_printarr(unsigned * arr, unsigned length); 

void main() {
    //printf_init();
    //uart_init();
    //timer_init();
    //mcp3008_init();
    //filters_init();
    //audio_init();
    //audio_send_waveform(&filters_reverb_wrapper,50000);
    //audio_send_filter(&filters_phaser);
    //main_printarr(filters_test(&filters_tremolo,10000),10000);
    //filters_printtremarr();
    interrupts_init(); 
    gpio_init(); 
    timer_init(); 
    console_init(NUM_ROWS, NUM_COLS); 
    keyboard_init(KEYBOARD_CLOCK, KEYBOARD_DATA);
    //audio_init();
    //audio_write_i16(resampled, PHASE);
    shell_init(console_printf); 
    interrupts_global_enable(); 
    //audio_init();
    shell_run(); 
    //audio_write_u8(sinewave, PHASE);
}

/*
void main_printarr(unsigned * arr, unsigned length) {
  for(int i = 0; i < length; i++) printf("%d ",arr[i]);
}
*/
