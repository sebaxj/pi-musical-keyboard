#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>

#define DELAY 2
#define CLOCK_DIVISOR 5
#define CYCLES 1024
#define RIGHTSHIFT (1024/CYCLES - 1)
/* Hardware abstraction functions for bare metal pulse-width
 * modulation (PWM) audio on the Raspberry Pi.
 *
 * NOTE: To use this library, you must first call timer_init().
 * Audio initialization requires timers for busy waiting.
 *
 * Author: Philip Levis <pal@cs.stanford.edu>
 * Date: September 10 2014
 */ 
void audio_init();
void audio_send_filter(unsigned (*function)(void));
void audio_write_u8 (const uint8_t  waveform[], unsigned dphase);
void audio_write_u16(const uint16_t waveform[], unsigned dphase);
void audio_write_i16(const  int16_t waveform[], unsigned dphase);
void audio_send_waveform(unsigned * (*function)(unsigned length), unsigned samples);
#endif

/*
 * Copyright (c) 2014 Stanford University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 * - Neither the name of the Stanford University nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL STANFORD
 * UNIVERSITY OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */