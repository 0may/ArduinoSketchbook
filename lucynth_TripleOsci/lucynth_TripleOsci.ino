/*
 * Software License Agreement (BSD License)
 *
 * Copyright (c) 2018 Oliver Mayer, Akademie der Bildenden Kuenste Nuernberg. 
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
 *  lucynth - light sensitive synthesizer (triple osci version)
 *  This sketch is for the lucynth using a single Arduino, which mixes 
 *  the signals of three oscillators to a single channel output.
 *****************************************************************************/

#include <MozziGuts.h>
#include <mozzi_rand.h>
#include <Oscil.h> // oscillator template
#include <tables/sin2048_int8.h> // sine table for oscillator
#include <tables/saw2048_int8.h> // saw table for oscillator
#include <RollingAverage.h>

#define CONTROL_RATE 64

// use: Oscil <table_size, update_rate> oscilName (wavetable), look in .h file of table #included above
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> sineA(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> sineB(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> sineC(SIN2048_DATA);

Oscil<SAW2048_NUM_CELLS, AUDIO_RATE> sawA(SAW2048_DATA);
Oscil<SAW2048_NUM_CELLS, AUDIO_RATE> sawB(SAW2048_DATA);
Oscil<SAW2048_NUM_CELLS, AUDIO_RATE> sawC(SAW2048_DATA);

Oscil<SAW2048_NUM_CELLS, AUDIO_RATE>* oscilA = NULL;
Oscil<SAW2048_NUM_CELLS, AUDIO_RATE>* oscilB = NULL;
Oscil<SAW2048_NUM_CELLS, AUDIO_RATE>* oscilC = NULL;


RollingAverage <int, 5> avgA0;
RollingAverage <int, 5> avgA1;
RollingAverage <int, 5> avgA2;
RollingAverage <int, 5> avgA3;

RollingAverage <byte, 3> avgNoteA;
RollingAverage <byte, 3> avgNoteB;
RollingAverage <byte, 3> avgNoteC;

float freqA = 100.0;
float freqB = 100.0;
float freqC = 100.0;
byte valX = 0;

byte noteOnA = 0;
byte noteOnB = 0;
byte noteOnC = 0;

byte prevNoteOnA = 0;
byte prevNoteOnB = 0;
byte prevNoteOnC = 0;

byte switchA = 0;
byte switchB = 0;
byte switchC = 0;

byte switchX = 0;

float* multipliersET;
byte* tones;

byte toneIdxA = 0;
byte toneIdxB = 0;
byte toneIdxC = 0;

byte asrA = 0;
byte asrB = 0; 
byte asrC = 0;

int asrCntA = 0;
int asrCntB = 0;
int asrCntC = 0;


void setup(){
  startMozzi(CONTROL_RATE); 

  multipliersET = new float[12];

  for (int i = 0; i < 12; i++) {
    multipliersET[i] = pow(2.0, i/12.0);    
  }

  tones = new byte[3];
  tones[0] = 0;
  tones[1] = 3;
  tones[2] = 7;

  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(13, OUTPUT);
  
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);

  if (sineMode()) {
    oscilA = &sineA;
    oscilB = &sineB;
    oscilC = &sineC; 
  }
  else {
    oscilA = &sawA;
    oscilB = &sawB;
    oscilC = &sawC; 
  }
  
  oscilA->setFreq(freqA);
  oscilB->setFreq(freqB);
  oscilC->setFreq(freqC);
}


void updateControl(){

  // ---- read potis ----
  float fA = avgA0.next(mozziAnalogRead(0))/1023.0;
  float fB = avgA1.next(mozziAnalogRead(1))/1023.0;
  float fC = avgA0.next(mozziAnalogRead(2))/1023.0;
  
  freqA = fA*fA*2943.246 + 30.0;
  freqB = fB*fB*2943.246 + 30.0;
  freqC = fC*fC*2943.246 + 30.0;
  valX = (byte)((avgA3.next(mozziAnalogRead(3)) >> 4) + 1);



  // ---- read switches ----

  switchA = (byte)(1-digitalRead(5));
  switchB = (byte)(1-digitalRead(6));
  switchC = (byte)(1-digitalRead(7));
  switchX = (byte)(1-digitalRead(8));

  // set previous notes
  prevNoteOnA = noteOnA;
  prevNoteOnB = noteOnB;
  prevNoteOnC = noteOnC;

  // read photo transistors
  if (switchX == 1) {
    noteOnA = avgNoteA.next((byte)digitalRead(2));
    noteOnB = avgNoteB.next((byte)digitalRead(3));
    noteOnC = avgNoteC.next((byte)digitalRead(4));
  }
  else {
    noteOnA = 1 - avgNoteA.next((byte)digitalRead(2));
    noteOnB = 1 - avgNoteB.next((byte)digitalRead(3));
    noteOnC = 1 - avgNoteC.next((byte)digitalRead(4));
  }

  if (switchA == 1) {
    if (noteOnA == 1 && prevNoteOnA == 0) {
      toneIdxA = rand(3);//(toneIdxA + 1) % 3;
    }
    oscilA->setFreq(freqA*multipliersET[tones[toneIdxA]]);
  }
  else {
    oscilA->setFreq(freqA);
  }
  
  if (switchB == 1) {
    if (noteOnB == 1 && prevNoteOnB == 0) {
      toneIdxB = rand(3);//(toneIdxB + 1) % 3;
    }
    oscilB->setFreq(freqB*multipliersET[tones[toneIdxB]]);
  }
  else {
    oscilB->setFreq(freqB);
  }
  
  if (switchC == 1) {   
    if (noteOnC == 1 && prevNoteOnC == 0) {
      toneIdxC = rand(3);//(toneIdxC + 1) % 3;
    }
    oscilC->setFreq(freqC*multipliersET[tones[toneIdxC]]);
  }
  else {
    oscilC->setFreq(freqC);
  }

}


int updateAudio(){

  if ((noteOnA == 1) && (asrA < 255)) {
    if (asrCntA >= valX) {
      asrA++;
      asrCntA = 0;
    }
    asrCntA++;
  }
  else if ((noteOnA == 0) && (asrA > 0)) {
    if (asrCntA >= valX) {
      asrA--;
      asrCntA = 0;
    }
    asrCntA++;
  }

  if ((noteOnB == 1) && (asrB < 255)) {
    if (asrCntB >= valX) {
      asrB++;
      asrCntB = 0;
    }
    asrCntB++;
  }
  else if ((noteOnB == 0) && (asrB > 0)) {
    if (asrCntB >= valX) {
      asrB--;
      asrCntB = 0;
    }
    asrCntB++;
  }

  if ((noteOnC == 1) && (asrC < 255)) {
    if (asrCntC >= valX) {
      asrC++;
      asrCntC = 0;
    }
    asrCntC++;
  }
  else if ((noteOnC == 0) && (asrC > 0)) {
    if (asrCntC >= valX) {
      asrC--;
      asrCntC = 0;
    }
    asrCntC++;
  }

  return (int)((((asrA*oscilA->next()) >> 8)
          + ((asrB*oscilB->next()) >> 8)
          + ((asrC*oscilC->next()) >> 8)
          ) >> 1);
}


void loop(){
  audioHook(); // required here
}


bool sineMode() {
  
  unsigned long t0 = mozziMicros();
  unsigned long dt = 0;

  byte changes = 0;
  byte s = (byte)digitalRead(8);
  byte slast;

  while (dt < 2000000) {
    
    slast = s;
    s = (byte)digitalRead(8);

    if (s != slast)
      changes++;

    if (dt % 400000 < 200000)
      digitalWrite(13, HIGH);
    else
      digitalWrite(13, LOW);

    dt = mozziMicros() - t0;
  }

  return (changes >= 2);
}


