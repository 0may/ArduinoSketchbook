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
 *  Sound synthesis using an oscillator with gain control 
 *  and controllable ADSR envelope for cutoff frequency of a low pass filter
 *****************************************************************************/

#include <MozziGuts.h>
#include <Oscil.h> // oscillator template
#include <tables/saw8192_int8.h> 
#include <tables/chum9_int8.h> // recorded audio wavetable
#include <RollingAverage.h>
#include <ADSR.h>
#include <LowPassFilter.h>

#define CONTROL_RATE 64

//Oscil<8192, AUDIO_RATE> oscil(SAW8192_DATA);
Oscil<CHUM9_NUM_CELLS, AUDIO_RATE> oscil(CHUM9_DATA);

ADSR <CONTROL_RATE, CONTROL_RATE> envelope;
LowPassFilter lpf;

RollingAverage <int, 16> avgA0; // average value of A0 over the last 16 reads
RollingAverage <int, 16> avgA1; // average value of A1 over the last 16 reads
RollingAverage <int, 16> avgA2; // average value of A1 over the last 16 reads

float gain = 0.0;         // Gain = average value of A0 normalized to range 0.0 to 1.0
unsigned int timeADR = 0;         // time value in ms for attack, decay and release
byte levelD = 0;          // decay level ( = sustain level)

uint8_t s0 = 0; // value of switch 0
uint8_t s1 = 0; // value of switch 1

uint8_t b0 = 0; // value of button 0
uint8_t b0Last = 0;

void setup(){
  // setup pins to read switch/button values with pullup resistors attached
  pinMode(2, INPUT); 
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  
  // Arduino Uno has internal pullup resistors (20kOhm) that may be used instead of wiring external resistors
  // setup pins to read switch/button values with pullup resistors attached
  //pinMode(2, INPUT_PULLUP);
  //pinMode(3, INPUT_PULLUP);  

  envelope.setLevels(255, 128, 128, 0);
  envelope.setTimes(500, 500, (unsigned int)-1, 500);

  lpf.setResonance(200);

  
  startMozzi(CONTROL_RATE);
}


void updateControl(){ 
  gain = avgA0.next(mozziAnalogRead(0)) / 1023.0;
  timeADR = (avgA1.next(mozziAnalogRead(1)) >> 1) + 50;
  levelD = (avgA2.next(mozziAnalogRead(2)) >> 2);

  s0 = digitalRead(2);
  s1 = digitalRead(3);
  b0Last = b0;
  b0 = 1 - digitalRead(4); // is 0 or 1 depending on button state

  envelope.setAttackLevel(levelD);
  envelope.setDecayLevel(levelD);
  envelope.setAttackTime(timeADR);
  envelope.setDecayTime(timeADR);
  envelope.setReleaseTime(timeADR);

  oscil.setFreq(1.0f + s0*1.0f + s1*2.0f);


  if (b0 && !b0Last) // button is pressed
    envelope.noteOn();
  else if (!b0 && !b0Last) // button is released
    envelope.noteOff();

  envelope.update();
  //lpf.setCutoffFreq(cutoffFreq);
  lpf.setCutoffFreq((byte)(255 - (envelope.next() >> 1) - 50)); // set cutoff frequency using the (inverted) ADSR envelope
}


int updateAudio(){
  return (int)(gain * (int)((char)lpf.next(oscil.next())));
}


void loop(){
  audioHook(); // required here
}


