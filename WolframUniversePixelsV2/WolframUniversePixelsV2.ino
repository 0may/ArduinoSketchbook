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
 * V2: color for state=0 configurable, smooth transition between colors when state changes
 ********************************************************************************************/

#include <Adafruit_NeoPixel.h>

#define PIN_LEDS 6
#define NUM_PIXELS 25
#define MAX_BRIGHTNESS 50

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, PIN_LEDS);

float    hsvOn[]      = {120, 100, 100};
float    hsvOff[]     = {120, 100, 0};
uint8_t* states       = NULL;
uint8_t* statesBuffer = NULL;
uint8_t  rules[8];


void setup() {  

  states       = new uint8_t[NUM_PIXELS];
  statesBuffer = new uint8_t[NUM_PIXELS];

  Serial.begin(9600);
  
  pixels.begin();
  pixels.setBrightness(50);

  initRules(110);
  serialPrintRules();
  
  initStates(4096);
  serialPrintStates();
  
  updatePixels();
}

  
void loop() {

  updateStates();

  for (float s = 0.0; s <= 1.0; s += 0.2) {
    delay(20);
    updatePixels(s);
  }
}


// between 0 and 2^NUM_PIXELS-1 (33,554,431‬ for NUM_PIXELS=25)
void initStates(uint32_t statesNumber) {
  for (uint32_t i = 0; i < NUM_PIXELS; i++) {
    states[i] = (0x1 & (statesNumber >> i));
  } 
}


void updateStates() {
  uint8_t r = 0;
  r += states[NUM_PIXELS-1];
  r += states[0] << 1;
  r += states[1] << 2;  
  statesBuffer[0] = rules[r];
 
  for (uint8_t i = 1; i < NUM_PIXELS-1; i++) {
    r = 0;
    r += states[i-1];
    r += states[i]   << 1;
    r += states[i+1] << 2;  
    statesBuffer[i] = rules[r];
  }

  r = 0;
  r += states[NUM_PIXELS-2];
  r += states[NUM_PIXELS-1] << 1;
  r += states[0]            << 2;  
  statesBuffer[NUM_PIXELS-1] = rules[r];

  uint8_t* statesPtr = states;
  states = statesBuffer;
  statesBuffer = statesPtr;
}


void updatePixels() {
  uint32_t clr = 0;
  byte* clrChannel = (byte*)&clr;
  
  for (uint8_t i = 0; i < NUM_PIXELS; i++) {
      
    if (states[i]) {
      HSV_to_RGB(hsvOn[0], hsvOn[1], hsvOn[2], clrChannel+2, clrChannel+1, clrChannel);
      pixels.setPixelColor(i, clr);
    }
    else {
      HSV_to_RGB(hsvOff[0], hsvOff[1], hsvOff[2], clrChannel+2, clrChannel+1, clrChannel);
      pixels.setPixelColor(i, 0);
    }  
  } 

  pixels.show();
}


void updatePixels(float step) {
  uint32_t clr = 0;
  byte* clrChannel = (byte*)&clr;
  
  for (uint8_t i = 0; i < NUM_PIXELS; i++) {
      
    if (states[i] == 1 && statesBuffer[i] == 0) {
      HSV_to_RGB(hsvOn[0]*step + hsvOff[0]*(1.0-step), hsvOn[1]*step + hsvOff[1]*(1.0-step), hsvOn[2]*step + hsvOff[2]*(1.0-step), clrChannel+2, clrChannel+1, clrChannel);
      pixels.setPixelColor(i, clr);
    }
    else if (states[i] == 0 && statesBuffer[i] == 1) {
      HSV_to_RGB(hsvOff[0]*step + hsvOn[0]*(1.0-step), hsvOff[1]*step + hsvOn[1]*(1.0-step), hsvOff[2]*step + hsvOn[2]*(1.0-step), clrChannel+2, clrChannel+1, clrChannel);
      pixels.setPixelColor(i, clr);
    }  
  } 

  pixels.show();
}


void initRules(uint8_t ruleNumber) {
  for (uint8_t i = 0; i < 8; i++) {
    rules[i] = (uint8_t)(0x1 & (ruleNumber >> i));
  } 
}


uint8_t rulesToInt() {
  uint8_t r = 0;
  
  for (uint8_t i = 0; i < 8; i++) {
    r += rules[i] << i;
  } 
  
  return r;
}


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


void serialPrintStates() {
  Serial.print("States: ");
  for (uint8_t i = 0; i < NUM_PIXELS; i++) {
    Serial.print(states[i]);
  }  
  Serial.println("");
}

void HSV_to_RGB(float h, float s, float v, byte *r, byte *g, byte *b)
{
  int i;
  float f,p,q,t;
  
  h = max(0.0, min(360.0, h));
  s = max(0.0, min(100.0, s));
  v = max(0.0, min(100.0, v));
  
  s /= 100;
  v /= 100;
  
  if(s == 0) {
    // Achromatic (grey)
    *r = *g = *b = round(v*255);
    return;
  }

  h /= 60; // sector 0 to 5
  i = floor(h);
  f = h - i; // factorial part of h
  p = v * (1 - s);
  q = v * (1 - s * f);
  t = v * (1 - s * (1 - f));
  switch(i) {
    case 0:
      *r = round(255*v);
      *g = round(255*t);
      *b = round(255*p);
      break;
    case 1:
      *r = round(255*q);
      *g = round(255*v);
      *b = round(255*p);
      break;
    case 2:
      *r = round(255*p);
      *g = round(255*v);
      *b = round(255*t);
      break;
    case 3:
      *r = round(255*p);
      *g = round(255*q);
      *b = round(255*v);
      break;
    case 4:
      *r = round(255*t);
      *g = round(255*p);
      *b = round(255*v);
      break;
    default: // case 5:
      *r = round(255*v);
      *g = round(255*p);
      *b = round(255*q);
    }
}



