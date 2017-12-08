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
 *  Sound synthesis using an oscillator with gain control and 
 *  control of pitches using Midi codes for chords C2 to B7 and 
 *  a switch/button for major/minor
 *****************************************************************************/

#include <MozziGuts.h>
#include <Oscil.h> // oscillator template
#include <tables/sin2048_int8.h> // sine table for oscillator
#include <tables/square_no_alias_2048_int8.h> // square table for oscillator
#include <tables/saw2048_int8.h> // saw table for oscillator
#include <tables/triangle2048_int8.h> // saw table for oscillator
#include <RollingAverage.h>
#include <mozzi_midi.h>

#define CONTROL_RATE 64

// use: Oscil <table_size, update_rate> oscilName (wavetable), look in .h file of table #included above
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> oscilA(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> oscilB(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> oscilC(SIN2048_DATA);

RollingAverage <int, 16> avgA0; // average value of A0 over the last 16 reads
RollingAverage <int, 16> avgA1; // average value of A1 over the last 16 reads
RollingAverage <int, 16> avgA2; // average value of A2 over the last 16 reads

float gain = 0.0;  // Gain = average value of A0 normalized to range 0.0 to 1.0
byte midiNote = 0; 
byte octave = 2;

uint8_t s0 = 0; // value of switch 0

void setup(){
  pinMode(2, INPUT); 
  
  startMozzi(CONTROL_RATE); // start with default control rate of 64
  oscilA.setFreq(mtof(midiNote));
  oscilB.setFreq(mtof(midiNote));
  oscilC.setFreq(mtof(midiNote));
}


void updateControl(){

  s0 = digitalRead(2);
  
  gain = avgA0.next(mozziAnalogRead(0)) / 1023.0f;
  octave = 2 + (byte)(avgA2.next(mozziAnalogRead(2)) / 1024.0f * 6.0f); // octave 2 to 7
  midiNote = 21 + octave*12 + (byte)(avgA1.next(mozziAnalogRead(1)) / 1024.0f * 12.0f); // Midi note 60 to 71 = C4 to B4
  oscilA.setFreq(mtof(midiNote));

  if (s0 == 0) // minor / Dur
    oscilB.setFreq(mtof(midiNote + 3));
  else // major / Moll
    oscilB.setFreq(mtof(midiNote + 4));
    
  oscilC.setFreq(mtof(midiNote + 7));
}


int updateAudio(){
  return (int)(gain * ((oscilA.next() + oscilB.next() + oscilC.next()) >> 2) );
}


void loop(){
  audioHook(); // required here
}
