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
 
#include <SPI.h>
#include <RH_RF95.h>
#include "LoRaPulserMsg.h"

//#define DEBUG

#define PULSER_ID 4

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
uint16_t ppmBuffer[3];
uint8_t ppmBufferIdx;
LoRaPulserMsg txMsg;
LoRaPulserMsg rxMsg;
bool sendMsg = false;
bool gotReply = true;
uint8_t rxBuf[RH_RF95_MAX_MESSAGE_LEN];
uint8_t rxBufLen;

uint8_t batcnt = 0;

unsigned long t_send = 0;
 
void setup() 
{
  pinMode(LED, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

#ifdef DEBUG
  Serial.begin(115200);
  while (!Serial) {
    delay(1);
  }
 
  delay(100);
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
  ppmBuffer[0] = 0;
  ppmBuffer[1] = 0;
  ppmBuffer[2] = 0;
  ppmBufferIdx = 0;

  rxBufLen = sizeof(rxBuf);
 
  txMsg.setPulserId(PULSER_ID);
  txMsg.setPPM(ppm);
}
 
void loop()
{
  delay(50); // Wait 50 ms between transmits, could also 'sleep' here!

       
  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage
 // measuredvbat = 3.8;
#ifdef DEBUG
  Serial.print("VBat: " ); Serial.println(measuredvbat);
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


  // mean filter and floor  poti values to account for unstable measurements (alternating values...) 
  ppmBuffer[ppmBufferIdx] = analogRead(A0);
  float ppmNew = (ppmBuffer[0] + ppmBuffer[1] + ppmBuffer[2])/3.0f;
  if (abs(ppmNew - ppm) >= 1)
    ppm = (uint16_t)(ppmNew+0.5);
  ppmBufferIdx = (ppmBufferIdx + 1) % 3;


  unsigned long t_now = millis();
  
  if (ppm != txMsg.getPPM() || t_now > t_send + 5000 || t_now < t_send) {
    txMsg.setPPM(ppm);
    txMsg.incrementMessageId();
    sendMsg = true;
    t_send = t_now;
  }

  if (!sendMsg && gotReply) {
    rf95.sleep();
  }

  if (sendMsg) {
  //  digitalWrite(LED, HIGH);

#ifdef DEBUG
    Serial.print("Sending message:"); // Send a message to rf95_server
    Serial.print(" p_id="); Serial.print(txMsg.getPulserId()); 
    Serial.print(" m_id="); Serial.print(txMsg.getMessageId()); 
    Serial.print(" ppm="); Serial.println(txMsg.getPPM());
    delay(10);
#endif

    rf95.send(txMsg.getData(), LORAPULSER_MSGLEN);

   // delay(10);
    rf95.waitPacketSent();

    gotReply = false;

   // digitalWrite(LED, LOW);

#ifdef DEBUG
  //  Serial.println("Done sending");
#endif

  }

  if (!gotReply) {
    
    // Now wait for a reply

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
          if (rxMsg.getPulserId() == PULSER_ID && rxMsg.getMessageId() == txMsg.getMessageId()) {

#ifdef DEBUG
            Serial.print("Got reply to message:");
            Serial.print(" p_id="); Serial.print(rxMsg.getPulserId()); 
            Serial.print(" m_id="); Serial.print(rxMsg.getMessageId()); 
            Serial.print(" ppm="); Serial.println(rxMsg.getPPM());
            Serial.print("RSSI: ");
            Serial.println(rf95.lastRssi(), DEC);    
#endif
            
            sendMsg = false;
            gotReply = true;
          }
        } 
      }


#ifdef DEBUG
      else
      {
        Serial.println("Receive failed");
        
      }
#endif
    }

#ifdef DEBUG    
    else
    {
      Serial.println("No reply, resending...");
    }
#endif
    
  }
  
}
