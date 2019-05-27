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

#define DEBUG

#define PULSER_ID         0
#define PPM_MAX           300

#define IRUPT_DT          1024/(double)8000000*255
#define IRUPTCNT_PPM_1    60.0/(IRUPT_DT)
#define IRUPTCNT_PPM_MAX  60.0/(IRUPT_DT * PPM_MAX)

#define LED               13

// for Feather32u4 RFM9x
#define RFM95_CS          8
#define RFM95_RST         4
#define RFM95_INT         7

// radio frequency
#define RF95_FREQ         433.0
 
// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);
 
LoRaPulserMsg rxMsg;
LoRaPulserMsg txMsg;
uint16_t ppmRaw;
uint16_t iruptCnt;
uint16_t iruptCntMax;

uint16_t iruptCntNoReceive;
uint16_t iruptCntNoReceiveLim;
 
void setup()
{
  pinMode(LED, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

#ifdef DEBUG
  Serial.begin(115200);
  while (!Serial) {
  //  delay(1);
  }
  //delay(100);

  Serial.println("Feather LoRa RX Test!");
#endif 
 
  // manual reset
  digitalWrite(RFM95_RST, LOW);
 
 // delay(10);
  digitalWrite(RFM95_RST, HIGH);
 // delay(10);
 
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

  ppmRaw = (uint16_t)(60.0*1022.0/(PPM_MAX-1));
  iruptCnt = 0;
  iruptCntMax = 0;

  iruptCntNoReceive = 0;
  iruptCntNoReceiveLim = 386;
  
  noInterrupts(); 
  TCCR0A = 0;
  TCCR0B = 0; 
  TCNT0 = 0;     
  OCR0A = 255;
  TCCR0A |= (1 << CS10); TCCR0A |= (1 << CS12);
  TCCR0B |= (1 << CS10); TCCR0B |= (1 << CS12);   // 1024 als Prescale-Wert spezifizieren
  TIMSK0 &= ~(1 << TOIE0); // deactivate Timer Overflow Interrupt
  TIMSK0 |= (1 << OCIE0A);
  
  setupPPM(ppmRaw);
  interrupts();
  
}


// interrupt handler
ISR(TIMER0_COMPA_vect)  //  TIMER0_OVF_vect
{
  TCNT0 = 0;                // Register mit 0 initialisieren   
  iruptCnt++;
  iruptCntNoReceive++;

  if (ppmRaw > 0 && iruptCnt == iruptCntMax) {
    digitalWrite(LED, digitalRead(LED) ^ 1); // LED ein und aus
    iruptCnt = 0;
  } 
}


void setupPPM(uint16_t ppm_raw) {
  if (ppm_raw > 0) {  
    float ppm = (ppm_raw-1)/1022.0 * (PPM_MAX - 1) + 1;
    iruptCntMax = (uint16_t)(60.0/(IRUPT_DT * ppm) + 0.5);
    iruptCnt = 0;
  }
}

 
void loop()
{
 // delay(100);
  if (rf95.available())
  {
    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
 
    if (rf95.recv(buf, &len))
    {

#ifdef DEBUG
      Serial.print("# Bytes received: "); Serial.println(len);
#endif

      if (len >= LORAPULSER_MSGLEN) {
        memcpy(rxMsg.getData(), buf, LORAPULSER_MSGLEN);
        memcpy(txMsg.getData(), buf, LORAPULSER_MSGLEN);
        
        if (rxMsg.getPulserId() == PULSER_ID) {

#ifdef DEBUG
          Serial.print("p_id: "); Serial.print(rxMsg.getPulserId()); 
          Serial.print("  m_id: "); Serial.print(rxMsg.getMessageId()); 
          Serial.print("  ppm: "); Serial.println(rxMsg.getPPM());
#endif

          iruptCntNoReceive = 0;

          if (rxMsg.getPPM() != ppmRaw) {
            ppmRaw = rxMsg.getPPM();
            setupPPM(ppmRaw);
          }

          rf95.send(txMsg.getData(), LORAPULSER_MSGLEN);
          rf95.waitPacketSent();  
        }
      }

    /*  
      RH_RF95::printBuffer("Received: ", buf, len);
      Serial.print("Got: ");
      Serial.println((char*)buf);
       Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);
 
      // Send a reply
      uint8_t data[] = "And hello back to you";
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
      Serial.println("Sent a reply");
      digitalWrite(LED, LOW);
      */
    }

#ifdef DEBUG    
    else
    {
      Serial.println("Receive failed");
    }
#endif

  }      
 
  if (ppmRaw > 0 && iruptCntNoReceive > iruptCntNoReceiveLim) {
    ppmRaw = 0;
    setupPPM(ppmRaw);

#ifdef DEBUG
    Serial.print("iruptCntNoReceive: "); Serial.println(iruptCntNoReceive); 
#endif
  }
}
