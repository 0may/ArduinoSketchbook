/*******************************************************************
Copyright (C) 2017 Oliver Mayer <mayer@adbk-nuernberg.de>

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://www.wtfpl.net/ for more details.
*******************************************************************/

#include <MIDI.h>

#define PIN_POTI1 0
#define PIN_POTI2 1

#define PIN_BUTTON1 4
#define PIN_BUTTON2 3
#define PIN_BUTTON3 2

#define PIN_LED1 7
#define PIN_LED2 6

#define PIN_RELAIS1 5
#define PIN_RELAIS2 8
#define PIN_RELAIS3 9
#define PIN_RELAIS4 10
#define PIN_RELAIS5 11
#define PIN_RELAIS6 12
#define PIN_RELAIS7 13

MIDI_CREATE_DEFAULT_INSTANCE();

enum SwitchMode {
  BY_NOTE, BY_CHANNEL
} switchMode;

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
  if (switchMode == BY_NOTE && channel == 1) {

    if (pitch == 48) // C3
      digitalWrite(PIN_RELAIS1, LOW);
    else if (pitch == 50) // D3
      digitalWrite(PIN_RELAIS2, LOW);
    else if (pitch == 52) // E3
      digitalWrite(PIN_RELAIS3, LOW);
    else if (pitch == 53) // F3
      digitalWrite(PIN_RELAIS4, LOW);
    else if (pitch == 55) // G3
      digitalWrite(PIN_RELAIS5, LOW);
    else if (pitch == 57) // A3
      digitalWrite(PIN_RELAIS6, LOW);
    else if (pitch == 59) // B3
      digitalWrite(PIN_RELAIS7, LOW);
      
  }
  else if (switchMode == BY_CHANNEL) { 
    
    if (channel == 1) 
      digitalWrite(PIN_RELAIS1, LOW);
    else if (channel == 2)
      digitalWrite(PIN_RELAIS2, LOW);
    else if (channel == 3)
      digitalWrite(PIN_RELAIS3, LOW);
    else if (channel == 4)
      digitalWrite(PIN_RELAIS4, LOW);
    else if (channel == 5)
      digitalWrite(PIN_RELAIS5, LOW);
    else if (channel == 6)
      digitalWrite(PIN_RELAIS6, LOW);
    else if (channel == 7)
      digitalWrite(PIN_RELAIS7, LOW);
      
  }
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
  if (switchMode == BY_NOTE && channel == 1) {

    if (pitch == 48) // C3
      digitalWrite(PIN_RELAIS1, HIGH);
    else if (pitch == 50) // D3
      digitalWrite(PIN_RELAIS2, HIGH);
    else if (pitch == 52) // E3
      digitalWrite(PIN_RELAIS3, HIGH);
    else if (pitch == 53) // F3
      digitalWrite(PIN_RELAIS4, HIGH);
    else if (pitch == 55) // G3
      digitalWrite(PIN_RELAIS5, HIGH);
    else if (pitch == 57) // A3
      digitalWrite(PIN_RELAIS6, HIGH);
    else if (pitch == 59) // B3
      digitalWrite(PIN_RELAIS7, HIGH);
    
  }
  else if (switchMode == BY_CHANNEL) { 
    
    if (channel == 1)
      digitalWrite(PIN_RELAIS1, HIGH);
    else if (channel == 2)
      digitalWrite(PIN_RELAIS2, HIGH);
    else if (channel == 3)
      digitalWrite(PIN_RELAIS3, HIGH);
    else if (channel == 4)
      digitalWrite(PIN_RELAIS4, HIGH);
    else if (channel == 5)
      digitalWrite(PIN_RELAIS5, HIGH);
    else if (channel == 6)
      digitalWrite(PIN_RELAIS6, HIGH);
    else if (channel == 7)
      digitalWrite(PIN_RELAIS7, HIGH);

  }
}

void setup()
{
    switchMode = BY_NOTE;
  
    pinMode(PIN_LED1, OUTPUT);
    pinMode(PIN_LED2, OUTPUT);
    digitalWrite(PIN_LED1, HIGH);
    digitalWrite(PIN_LED2, HIGH);


    pinMode(PIN_BUTTON1, INPUT_PULLUP);
    pinMode(PIN_BUTTON2, INPUT_PULLUP);
    pinMode(PIN_BUTTON3, INPUT_PULLUP);


    pinMode(PIN_RELAIS1, OUTPUT);
    pinMode(PIN_RELAIS2, OUTPUT);
    pinMode(PIN_RELAIS3, OUTPUT);
    pinMode(PIN_RELAIS4, OUTPUT);
    pinMode(PIN_RELAIS5, OUTPUT);
    pinMode(PIN_RELAIS6, OUTPUT);
    pinMode(PIN_RELAIS7, OUTPUT);
    digitalWrite(PIN_RELAIS1, HIGH);
    digitalWrite(PIN_RELAIS2, HIGH);
    digitalWrite(PIN_RELAIS3, HIGH);
    digitalWrite(PIN_RELAIS4, HIGH);
    digitalWrite(PIN_RELAIS5, HIGH);
    digitalWrite(PIN_RELAIS6, HIGH);
    digitalWrite(PIN_RELAIS7, HIGH);
  
    MIDI.begin(MIDI_CHANNEL_OMNI);
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
}

void loop()
{
  
    MIDI.read();   
    
}
