#include <stdint.h> 
#include "console.h"
#include "interrupts.h"
#include "keyboard.h"
#include "shell.h"
#include "timer.h"
#include "uart.h"
#include "audio.h"
#include "piano.h"

#define NUM_ROWS 20 
#define NUM_COLS 40 

void main(void) {

  // initialize modules
  interrupts_init();
  gpio_init();
  timer_init();
  uart_init();
  keyboard_init(KEYBOARD_CLOCK, KEYBOARD_DATA);
  console_init(NUM_ROWS, NUM_COLS);
  piano_init(NUM_ROWS, NUM_COLS);
  audio_init();
  shell_init(console_printf);
  interrupts_global_enable(); // everything fully initialized, now turn on interrupts

  shell_run();
}
