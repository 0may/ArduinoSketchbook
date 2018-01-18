/*  Example of playing a sampled sound,
    using Mozzi sonification library.
  
    Demonstrates one-shot samples scheduled
    with EventDelay.
  
    Circuit: Audio output on digital pin 9 on a Uno or similar, or
    DAC/A14 on Teensy 3.1, or 
    check the README or http://sensorium.github.com/Mozzi/
  
    Mozzi help/discussion/announcements:
    https://groups.google.com/forum/#!forum/mozzi-users
  
    Tim Barrass 2012, CC by-nc-sa.
*/

#include <MozziGuts.h>
#include <Sample.h> // Sample template
#include <samples/impulse/sine_impulse25.h>
#include <EventDelay.h>
#include <RollingAverage.h>
#include <mozzi_rand.h>

#define CONTROL_RATE 64

// use: Sample <table_size, update_rate> SampleName (wavetable)
//Sample <UOU_NUM_CELLS, AUDIO_RATE> aSample(UOU_DATA);
Sample <IMPULSE25_NUM_CELLS, AUDIO_RATE> aSample(IMPULSE25_DATA);

RollingAverage <int, 16> avgA0; // average value of A0 over the last 16 reads
RollingAverage <int, 16> avgA1; // average value of A1 over the last 16 reads
RollingAverage <int, 16> avgA2; // average value of A2 over the last 16 reads

// for scheduling sample start
EventDelay kTriggerDelay;

float gain = 0.0;
float freq = 0.0;
int tdelay = 1500;


void setup(){
  startMozzi(CONTROL_RATE);
  aSample.setFreq((float) IMPULSE25_SAMPLERATE / (float) IMPULSE25_NUM_CELLS * 3); // play at the speed it was recorded
  kTriggerDelay.set(1500); // 1500 msec countdown, within resolution of CONTROL_RATE
}


void updateControl(){
  gain = avgA0.next(mozziAnalogRead(0)) / 1023.0;
  freq = (float) IMPULSE25_SAMPLERATE / (float) IMPULSE25_NUM_CELLS * (avgA2.next(mozziAnalogRead(2))/1023.0*5.0 + 0.5);
  
  aSample.setFreq(freq);
  
  tdelay = 4110 - avgA1.next(mozziAnalogRead(1))*4;
  
  if(kTriggerDelay.ready()){
    
    kTriggerDelay.set(tdelay);
    
    aSample.start();
    kTriggerDelay.start();
  }
}


int updateAudio(){
  return (int) (gain * aSample.next() * 6.0);
}


void loop(){
  audioHook();
}



