/*
 * Software License Agreement (BSD License)
 *
 * Copyright (c) 2019 Oliver Mayer, Akademie der Bildenden Kuenste Nuernberg. 
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
 
/********************************************************************************************
 * Implementation of Wolfram's one-dimensional universe with LEDs (Adafruit NeoPixels)
 * https://de.wikipedia.org/wiki/Zellul%C3%A4rer_Automat#Wolframs_eindimensionales_Universum
 ********************************************************************************************/

#include <Adafruit_NeoPixel.h>

#define PIN_LEDS 6          // output pin for LED strip control
#define NUM_PIXELS 25       // number of LEDs on the strip
#define MAX_BRIGHTNESS 50   // maximum LED brightness

// init NeoPixel object for LED control
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, PIN_LEDS);

uint32_t onColor      = 0x00ff00;  // color of a LED when state is 1
uint8_t* states       = NULL;      // array of state variables
uint8_t* statesBuffer = NULL;      // another array of state variables for updating
uint8_t  rules[8];                 // rules of the universe


void setup() {  

  // create arrays of state variables
  states       = new uint8_t[NUM_PIXELS];  
  statesBuffer = new uint8_t[NUM_PIXELS];

  // init serial 
  Serial.begin(9600);

  // init NeoPixels object
  pixels.begin();
  pixels.setBrightness(MAX_BRIGHTNESS);

  // specify rules by a number from 0 to 255
  initRules(110);
  serialPrintRules();

  // specify initial states by a number from 0 to 33,554,4321
  initStates(4096);
  serialPrintStates();
  
  updatePixels();
}

  
void loop() {
  
  delay(200);

  updateStates();
  updatePixels();

}


// Initialize state array by the Bits of the number specified.
// statesNumber must be in the range 0 and 2^NUM_PIXELS - 1 (33,554,4321‬ for NUM_PIXELS=25)
void initStates(uint32_t statesNumber) {
  for (uint32_t i = 0; i < NUM_PIXELS; i++) {
    states[i] = (0x1 & (statesNumber >> i));
  } 
}


void updateStates() {
  
  // each element in the state array is updated depending on its own state and its left and 
  // right neighbor state according to the rules. The new states are stored in the 'statesBuffer'
  // array. After updating the 'statesBuffer' array is switched with the 'states' array.
  
  uint8_t r = 0;

  // left neighbor of the first state in the array ist the last state in the array
  // as the dimension of the universe is ring-like
  r += states[NUM_PIXELS-1];
  r += states[0] << 1;
  r += states[1] << 2;  
  statesBuffer[0] = rules[r];

  // states 1 to NUM_PIXELS-2
  for (uint8_t i = 1; i < NUM_PIXELS-1; i++) {
    r = 0;
    r += states[i-1];
    r += states[i]   << 1;
    r += states[i+1] << 2;  
    statesBuffer[i] = rules[r];
  }

  // right neighbor of the last state in the array ist the first state in the array
  r = 0;
  r += states[NUM_PIXELS-2];
  r += states[NUM_PIXELS-1] << 1;
  r += states[0]            << 2;  
  statesBuffer[NUM_PIXELS-1] = rules[r];

  // switch 'statesBuffer' and 'states' array
  uint8_t* statesPtr = states;    // get a pointer on the 'states' array
  states = statesBuffer;          // let 'states' point to the 'statesBuffer' array
  statesBuffer = statesPtr;       // let 'statesBuffer' point to the old 'states' array
}


void updatePixels() {
  
  for (uint8_t i = 0; i < NUM_PIXELS; i++) {
      
    if (states[i]) {
      pixels.setPixelColor(i, onColor);
    }
    else {
      pixels.setPixelColor(i, 0);
    }  
  } 

  pixels.show();
}


// init rules array by specifying a rule number in the range of 0 and 255.
void initRules(uint8_t ruleNumber) {
  for (uint8_t i = 0; i < 8; i++) {
    rules[i] = (uint8_t)(0x1 & (ruleNumber >> i));
  } 
}


// get the rule number associated to the current rules
uint8_t rulesToInt() {
  uint8_t r = 0;
  
  for (uint8_t i = 0; i < 8; i++) {
    r += rules[i] << i;
  } 
  
  return r;
}


// print rules to Serial
void serialPrintRules() {
  uint8_t rn = rulesToInt();
  
  Serial.print("Rule number: ");
  Serial.println(rn);

  for (uint8_t i = 0; i < 8; i++) {
    Serial.print(0x1 & (i >> 2));
    Serial.print(0x1 & (i >> 1));
    Serial.print(0x1 & i);
    Serial.print(": ");
    Serial.println(rules[i]);
  } 
}


// print states to Serial
void serialPrintStates() {
  Serial.print("States: ");
  for (uint8_t i = 0; i < NUM_PIXELS; i++) {
    Serial.print(states[i]);
  }  
  Serial.println("");
}

