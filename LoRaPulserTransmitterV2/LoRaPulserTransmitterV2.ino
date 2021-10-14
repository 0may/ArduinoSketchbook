/*
 * Software License Agreement (BSD License)
 *
 * Copyright (c) 2021 Oliver Mayer
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
 
#include <SPI.h>
#include <RH_RF95.h>
#include <MIDIUSB.h>
#include "LoRaPulserMsgV2.h"

//#define DEBUG

#define PULSER_ID 1

#define BUTTON 11
#define LED 13
#define VBATPIN A9
 
// for feather32u4 
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7

// radio frequency
#define RF95_FREQ 433.0
 
// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);


uint16_t ppm;
uint16_t ppmPrev;
uint16_t ppmBuffer[9];
uint8_t ppmBufferIdx;
LoRaPulserMsg txMsg;
LoRaPulserMsg rxMsg;
bool sendMsg = false;
bool gotReply = true;
uint8_t rxBuf[RH_RF95_MAX_MESSAGE_LEN];
uint8_t rxBufLen;

uint8_t batcnt = 0;

unsigned long t_send = 0;
unsigned long t_now = 0;

uint8_t buttonStatePrev = HIGH;
uint8_t buttonState;

midiEventPacket_t midiEvent;

bool midiMode = false;

 
void setup() 
{
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(115200);
  delay(500);
 
#ifdef DEBUG
  Serial.println("Feather LoRa TX Test!");
#endif

  
  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
 
  while (!rf95.init()) {
    
#ifdef DEBUG
    Serial.println("LoRa radio init failed");
#endif

    while (1);
  }
  
#ifdef DEBUG
  Serial.println("LoRa radio init OK!");
#endif

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {

#ifdef DEBUG
    Serial.println("setFrequency failed");
#endif

    while (1);
  }

#ifdef DEBUG
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
#endif
  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
 
  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);

  ppm = 0;
  ppmPrev = 0;
  ppmBuffer[0] = 0;
  ppmBuffer[1] = 0;
  ppmBuffer[2] = 0;
  ppmBufferIdx = 0;

  rxBufLen = sizeof(rxBuf);

  midiMode = (UDADDR & _BV(ADDEN)); // check if connected to a host computer by USB

  if (midiMode) {
    Serial.println("### Running Midi mode ###");
    setupMidiMode();
  }
  else {
    Serial.println("### Running manual mode ###");
    setupManualMode();
  }
}


void loop()
{
  if (midiMode) 
    loopMidiMode();
  else
    loopManualMode();
}

void setupManualMode() {
  for (uint8_t i = 0; i < 3; i++) {
    digitalWrite(LED, HIGH);
    delay(250);
    digitalWrite(LED, LOW);
    delay(250);
    digitalWrite(LED, HIGH);
    delay(250);
    digitalWrite(LED, LOW);
    delay(250);
    digitalWrite(LED, HIGH);
    delay(250);
    digitalWrite(LED, LOW);
    delay(750);
  }
  
  txMsg.setPulserMask(0);
  txMsg.setPulser(PULSER_ID, true);
  txMsg.setPPM(ppm);
}


void setupMidiMode() {
  for (uint8_t i = 0; i < 3; i++) {
    digitalWrite(LED, HIGH);
    delay(250);
    digitalWrite(LED, LOW);
    delay(250);
    digitalWrite(LED, HIGH);
    delay(250);
    digitalWrite(LED, LOW);
    delay(750);
  }
  
  txMsg.setPulserMask(0);
  txMsg.setPPM(ppm);
}



void loopManualMode() {

  batteryIndicator();

  buttonState = digitalRead(BUTTON);

  if (buttonState == HIGH && buttonStatePrev == LOW) {
    buttonStatePrev = HIGH;
  }
  else if (buttonState == LOW && buttonStatePrev == HIGH) {
    
    txMsg.setInstantPulse();
    sendMessage(0);
    buttonStatePrev = LOW;
    
#ifdef DEBUG
    Serial.println("SHOT!!!!!");
#endif
 
  }
  else {
    
    // mean filter and floor  poti values to account for unstable measurements (alternating values...) 
    ppmBuffer[ppmBufferIdx] = analogRead(A0);
    float ppmNew = (ppmBuffer[0] + ppmBuffer[1] + ppmBuffer[2] + ppmBuffer[3] + ppmBuffer[4])/5.0f;
    if (/*abs(ppmNew - ppm) > 1 || */
      (ppmBuffer[0] == ppmBuffer[1] 
      && ppmBuffer[0] == ppmBuffer[2]
      && ppmBuffer[0] == ppmBuffer[3]
      && ppmBuffer[0] == ppmBuffer[4]
      && ppmBuffer[0] == ppmBuffer[5]
      && ppmBuffer[0] == ppmBuffer[6]
      && ppmBuffer[0] == ppmBuffer[7]
      && ppmBuffer[0] == ppmBuffer[8])) 
    {
      ppm = ppmBuffer[0];//(uint16_t)(ppmNew+0.5);
    }
    ppmBufferIdx = (ppmBufferIdx + 1) % 9;
  
    t_now = millis();
    
    if (ppm != ppmPrev || t_now > t_send + 5000 || t_now < t_send) {
      
#ifdef DEBUG
    Serial.print("new ppm: "); Serial.println(ppm);
#endif 
      
      ppmPrev = ppm;
      txMsg.setPPM(ppm);
      t_send = t_now;
      sendMessage(0);
    }
  }
}



void loopMidiMode() {

  midiEvent = MidiUSB.read();

  if (midiEvent.header != 0) {
  
#ifdef DEBUG     
    uint8_t cnt = 0;
#endif

    txMsg.setPulserMask(0);
  
    do {

      if (midiEvent.header == 0x9) { // note on, channel not considered

        uint8_t p_id = (midiEvent.byte2 & 0x7F) - 60;  // pulser ids start from midi note 60 (id 0 = midi 60, id 1 = midi 61 ...)
        txMsg.setPulser(p_id, true);
      }

#ifdef DEBUG     
      cnt++;
      Serial.print("Received: ");
      Serial.print(midiEvent.header);
      Serial.print("-");
      Serial.print(midiEvent.byte1);
      Serial.print("-");
      Serial.print(midiEvent.byte2);
      Serial.print("-");
      Serial.println(midiEvent.byte3);
#endif

      midiEvent = MidiUSB.read();
      
    } while (midiEvent.header != 0);

#ifdef DEBUG
  Serial.print("Num Midi Events received: ");
  Serial.println(cnt);
#endif

    if (txMsg.getPulserMask() != 0) {
      txMsg.setInstantPulse();
      sendMessage(0);
      digitalWrite(LED, HIGH);
      delay(1);
      digitalWrite(LED, LOW);
    }  
  }  
}



void sendMessage(uint8_t retries) {
  
  txMsg.incrementMessageId();

#ifdef DEBUG
    Serial.print("Sending message:"); // Send a message to rf95_server
    Serial.print(" p_mask="); Serial.print(txMsg.getPulserMask()); 
    Serial.print(" m_id="); Serial.print(txMsg.getMessageId()); 
    Serial.print(" ppm="); Serial.println(txMsg.getPPM());
#endif

  rf95.send(txMsg.getData(), LORAPULSER_MSGLEN);
  rf95.waitPacketSent();


  while (retries > 0) {
    
#ifdef DEBUG
    Serial.println("Waiting for reply...");
#endif
    
    if (rf95.waitAvailableTimeout(50))
    { 
      // Should be a reply message for us now   
      if (rf95.recv(rxBuf, &rxBufLen))
      {
        if (rxBufLen >= LORAPULSER_MSGLEN) {
          memcpy(rxMsg.getData(), rxBuf, LORAPULSER_MSGLEN);
          if (rxMsg.getPulserMask() == txMsg.getPulserMask() && rxMsg.getMessageId() == txMsg.getMessageId()) {

#ifdef DEBUG
            Serial.print("Sending success: Got reply to message:");
            Serial.print(" p_mask="); Serial.print(rxMsg.getPulserMask()); 
            Serial.print(" m_id="); Serial.print(rxMsg.getMessageId()); 
            Serial.print(" ppm="); Serial.println(rxMsg.getPPM());
            Serial.print("RSSI: ");
            Serial.println(rf95.lastRssi(), DEC);    
#endif

            retries = 0;
          }
        } 
      }
    }

    if (retries > 0) {
#ifdef DEBUG
        Serial.println("Failed to get response. Retrying to send...");
#endif
        
        retries--;
        
        rf95.send(txMsg.getData(), LORAPULSER_MSGLEN);
        rf95.waitPacketSent();
    }
  }
}



void batteryIndicator() {

  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage
 // measuredvbat = 3.8;
#ifdef DEBUG
 // Serial.print("VBat: " ); Serial.println(measuredvbat);
#endif

 // batcnt = constrain(measuredvbat-3.2, 0.0, 0.7)/0.7*9 + 1;


  if (measuredvbat <= 3.2) {
    if (batcnt == 0) {
      digitalWrite(LED, digitalRead(LED) ^ 1);
    }

    batcnt = (batcnt + 1) % 2;
  }
  else if (measuredvbat <= 3.9) {
     if (batcnt == 0) {
      digitalWrite(LED, digitalRead(LED) ^ 1);
    }

    batcnt = (batcnt + 1) % 20;   
  }
  else {
    digitalWrite(LED, HIGH);
  }
}
