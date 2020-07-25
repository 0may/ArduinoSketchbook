/*
 * Software License Agreement (BSD License)
 *
 * Copyright (c) 2020 Oliver Mayer.
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
 *  DAF Pulser, version 2 with impulse on button press
 *****************************************************************************/


#include <MozziGuts.h>
#include <Sample.h> 
#include <samples/impulse/sine_impulse25.h>
#include <RollingAverage.h>
#include <mozzi_rand.h>

#define CONTROL_RATE 128

Sample <IMPULSE25_NUM_CELLS, AUDIO_RATE> aSample(IMPULSE25_DATA);

RollingAverage <int, 16> avgA0; // average value of A0 over the last 16 reads
RollingAverage <int, 16> avgA1; // average value of A1 over the last 16 reads

float freq = 0.0;
int tdelay = 6000;
int infdelay = 4*1023 + 20;

bool buttonPressed = false;
bool buttonPressedPrev = false;
bool doPulse = false;

unsigned long deltaT = 0;
unsigned long currentT = 0;
unsigned long lastT = 0;


void setup(){
 //  Serial.begin(9600);

  pinMode(11, INPUT_PULLUP);
  
  startMozzi(CONTROL_RATE);
  aSample.setFreq(0); 
}


void updateControl(){

  //int val = mozziAnalogRead(1);
  //Serial.println(val);

  // set impulse sampling rate
  freq = (float) IMPULSE25_SAMPLERATE / (float) IMPULSE25_NUM_CELLS * ((float)avgA1.next(mozziAnalogRead(1))/1023.0*2.5 + 0.4);
  aSample.setFreq(freq);

  // set impulse time spacing
  tdelay = 4*avgA0.next(mozziAnalogRead(0)) + 20;


  // test for button press and pulse if necessary
  if (digitalRead(11) == LOW) {
    if (buttonPressedPrev == false) {
      buttonPressed = true;
      doPulse = true;
    }
  }
  else if (buttonPressedPrev == true) {
    buttonPressed = false;
  }
  buttonPressedPrev = buttonPressed;


  // compute time and pulse if necessary
  currentT = mozziMicros();

  if (currentT >= lastT) {
    deltaT += (unsigned long)((currentT-lastT)/1000.0);
  }
  else {
    deltaT += (unsigned long)((((unsigned long)-1) - lastT + currentT)/1000.0);  
  }
  lastT = currentT;


  if (tdelay < infdelay && deltaT > (unsigned long)tdelay) {
    doPulse = true;
    deltaT = 0;
  }

  // start impulse sample
  if (doPulse) {
    aSample.start();
    doPulse = false;
  }
}


int updateAudio(){
  return (int) (aSample.next() * 3 );//16);
}


void loop(){
  audioHook();
}




