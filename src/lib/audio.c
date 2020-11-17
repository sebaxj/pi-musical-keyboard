#include "gpio.h"
#include "timer.h"
#include "audio.h"
#include "printf.h"
#include "filters.h"
#include "pwm.h"
#include "keyboard.h"

/*
 * 				  AUDIO.C
 *  A file that allows a user to control audio output from the PWM of the pi.
 *  Has two modes -- SD / CM, the former is well-balanced PWM and the latter
 *  is blocked PWM.
 *
 * Courtesy of Anastasios Angelopoulos, CS107e Spring 2017.
 * An improved version of the audio library provided by the course.
 * Basically, these allow for arbitrary waveforms to be sent through the PWM
 * (nothing is precompiled).  I think audio.c works by taking in a function pointer
 * which returns a processed sample for output at each clock cycle.
 * There's a sample rate issue with the PWM, but for low sounds, it doesn't matter too much.
 *
 *
 *  @author aangelopouls hleou plevin pathanrahan
 *  @date 12 June 2017
 * 
 *  @revised Sebastian James
 *  @date 16 November 2020
*/

/***********************************************************************/

/*
   Initialize the pwm for audio.
   - base clock is 9600000
   - pwm range is 256  for a sample clock rate of 37500 Khz
   - writing a 256 sample waveform will repeat sound at 37500/256 = 146.5 Hz
*/
void audio_init() {
    // initialize audio jack for PWN
    gpio_set_function(GPIO_PIN40, GPIO_FUNC_ALT0);
    gpio_set_function(GPIO_PIN45, GPIO_FUNC_ALT0);
    timer_delay_ms(DELAY);

    // initialize PWN
    pwm_init();
    pwm_set_clock(19200000/CLOCK_DIVISOR); // 9600000 Hz
    timer_delay_ms(DELAY);

    pwm_set_mode(0, PWM_SIGMADELTA); 
    pwm_set_mode(1, PWM_SIGMADELTA);

    pwm_set_fifo(0, 1); 
    pwm_set_fifo(1, 1);

    pwm_enable(0); 
    pwm_enable(1);

    // pwm range is 256 cycles
    pwm_set_range(0, 0x100);
    pwm_set_range(1, 0x100);
    timer_delay_ms(DELAY);
}

/* 
   These functions transmit a wave to the RPi audio jack 
   as a pulse-width-modulated signal.
   
   The waveform array contains 256 entries.
   There are 3 functions depending on the type of the
   waveform array: u8, u16, i16.
   phase is a 32-bit quantity. The upper 8-bits are used as an
   index into the wavefrom array and selects one of the 256 samples.
   
   dphase is added to phase after each sample is output.  
   Setting dphase=(1<<24) will output one sample per interval. 
   Running the pwm at 9600000 / 256 samples / 256 range results
   in the waveform being output at 146.5 Hz.
   Setting dphase=(2<<24) will skip one sample per interval,
   and will double the frequency of the output to 293Hz.
   
   These functions do not return.
*/

// play audio for specific duration
void audio_write_u8(const uint8_t waveform[], unsigned dphase, unsigned int duration) 
{
    unsigned phase = 0;
    unsigned int current_ticks = timer_get_ticks();
    while(timer_get_ticks() - current_ticks < duration * TONE_CYCLE) {
        unsigned status = pwm_get_status();
        if (!(status & PWM_FULL1)) {
            unsigned angle = phase >> 24;
            uint8_t pcm = waveform[angle];
            pwm_write( pcm ); // output to channel 0
            pwm_write( pcm ); // output to channel 1
            phase += dphase; 
        }
    }
}

void audio_write_u16(const uint16_t waveform[], unsigned dphase, unsigned int duration) 
{
    unsigned phase = 0;
    unsigned int current_ticks = timer_get_ticks();
    while(timer_get_ticks() - current_ticks < duration * TONE_CYCLE) {
        unsigned status = pwm_get_status();
        if (!(status & PWM_FULL1)) {
            unsigned angle = phase >> 24;
            uint8_t pcm = waveform[angle] >> 8;
            pwm_write( pcm ); // output to channel 0
            pwm_write( pcm ); // output to channel 1
            phase += dphase; 
        }
    }
}

void audio_write_i16(const int16_t waveform[], unsigned dphase, unsigned int duration) 
{
    unsigned phase = 0;
    unsigned int current_ticks = timer_get_ticks();
    while(timer_get_ticks() - current_ticks < duration * TONE_CYCLE) {
        unsigned status = pwm_get_status();
        if (!(status & PWM_FULL1)) {
            unsigned angle = phase >> 24;
            unsigned wave = waveform[angle] - INT16_MIN;
            uint8_t pcm = wave>>8;
            pwm_write( pcm ); // output to channel 0
            pwm_write( pcm ); // output to channel 1
            phase += dphase; 
        }
    }
}

/*
 *  Sends a filtered signal out through the RPI PWM.
 *  Just put in a function that returns the next value to output.
 */
void audio_send_filter(unsigned (*function)(void)) {
    unsigned read = function();
    while(1) {
      int status =  pwm_get_status();
      if (!(status & PWM_FULL1)) {
	    pwm_write(read>>RIGHTSHIFT);
	    read = function();
      }
    }
}

void audio_send_waveform(unsigned * (*function)(unsigned length), unsigned samples) {
  unsigned * toPlay = function(samples);
  printf("Playing %d samples\n",samples);
  int i = 0;
  unsigned read = toPlay[i];
  while(i < samples) {
    int status = pwm_get_status();
    if(!(status & PWM_FULL1)) {
      pwm_write(read>>RIGHTSHIFT);
      read = toPlay[++i];
    }
  }
}
