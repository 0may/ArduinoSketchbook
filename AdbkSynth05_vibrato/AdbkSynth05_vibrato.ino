/*
 * Software License Agreement (BSD License)
 *
 * Copyright (c) 2017 Oliver Mayer, Akademie der Bildenden Kuenste Nuernberg. 
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*****************************************************************************  
 *  Sound synthesis using an oscillator with gain and frequency control 
 *  and switchable oscillators (sine, square, sawtooth, triangle)
 *  and a vibrato effect
 *****************************************************************************/

#include <MozziGuts.h>
#include <Oscil.h> // oscillator template
#include <tables/sin2048_int8.h> // sine table for oscillator
#include <tables/square_no_alias_2048_int8.h> // square table for oscillator
#include <tables/saw2048_int8.h> // saw table for oscillator
#include <tables/triangle2048_int8.h> // saw table for oscillator
#include <RollingAverage.h>

#define CONTROL_RATE 64

// use: Oscil <table_size, update_rate> oscilName (wavetable), look in .h file of table #included above
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> oscilSine(SIN2048_DATA);
Oscil<SQUARE_NO_ALIAS_2048_NUM_CELLS, AUDIO_RATE> oscilSquare(SQUARE_NO_ALIAS_2048_DATA);
Oscil<SAW2048_NUM_CELLS, AUDIO_RATE> oscilSaw(SAW2048_DATA);
Oscil<TRIANGLE2048_NUM_CELLS, AUDIO_RATE> oscilTriangle(TRIANGLE2048_DATA);

Oscil<SIN2048_NUM_CELLS, AUDIO_RATE>* pOscil; // this is a POINTER to a Oscil object

// low frequency oscillator (LFO) for vibrato
// (sound oscillator frequency corresponds to the integral of the vibrato oscillator!!)
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> vibrato(SIN2048_DATA);
//Oscil<TRIANGLE2048_NUM_CELLS, AUDIO_RATE> vibrato(TRIANGLE2048_DATA);

RollingAverage <int, 16> avgA0; // average value of A0 over the last 16 reads
RollingAverage <int, 16> avgA1; // average value of A1 over the last 16 reads

float gain = 0.0;  // Gain = average value of A0 normalized to range 0.0 to 1.0
float freq = 0.0;  // Frequency = average value of A1 normalized to range 0.0 to 1.0

uint8_t s0 = 0; // value of switch/button 0
uint8_t s1 = 0; // value of switch/button 1

void setup(){
  // setup pins to read switch/button values with pullup resistors attached
  pinMode(2, INPUT); 
  pinMode(3, INPUT);
  
  // Arduino Uno has internal pullup resistors (20kOhm) that may be used instead of wiring external resistors
  // setup pins to read switch/button values with pullup resistors attached
  //pinMode(2, INPUT_PULLUP);
  //pinMode(3, INPUT_PULLUP);
  
  startMozzi(CONTROL_RATE); // start with default control rate of 64

  selectOscil();
  pOscil->setFreq(350); // set initial sound oscillator frequency
  vibrato.setFreq(3);  // set tremolo frequency
}


void updateControl(){ 
  gain = avgA0.next(mozziAnalogRead(0)) / 1023.0;
  freq = avgA1.next(mozziAnalogRead(1)) / 1023.0 * 2000 + 120;

  s0 = digitalRead(2);
  s1 = digitalRead(3);

  selectOscil();
  pOscil->setFreq(freq);
}


int updateAudio(){
  Q15n16 vibratoValue = (Q15n16) 1024*vibrato.next();
  
  return (int)(gain * pOscil->phMod(vibratoValue));
}


void loop(){
  audioHook(); // required here
}


void selectOscil() {
  if (s0 == 0 && s1 == 0)
    pOscil = &oscilSine;
  else if (s0 == 0 && s1 == 1)
    pOscil = &oscilSquare;
  else if (s0 == 1 && s1 == 0)
    pOscil = &oscilSaw;
  else
    pOscil = &oscilTriangle;
}

