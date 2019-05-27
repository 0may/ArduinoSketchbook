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

#ifndef LORAPULSERMSG_H
#define LORAPULSERMSG_H

#define LORAPULSER_MSGLEN 5

class LoRaPulserMsg {
  
public:
  uint8_t data[LORAPULSER_MSGLEN];   // message array

public:
  uint8_t* pulserId;    // pulser id (msg[0])
  uint16_t* msgId;      // message id (msg[1-2]
  uint16_t* ppm;  // pulses per minute (msg[3-4])

public:
  LoRaPulserMsg() {
    pulserId = data;
    msgId = (uint16_t*)(data+1);
    ppm = (uint16_t*)(data+3);

    setPulserId(0);
    setMessageId(0);
    setPPM(0);
  }

  void setPulserId(uint8_t pulserId) {
    *(this->pulserId) = pulserId;
  }

  void setMessageId(uint16_t msgId) {
    *(this->msgId) = msgId;
  }
  
  void incrementMessageId() {
    if (*(msgId) == (uint16_t)-1)
      *(this->msgId) = 0;
    else
      *(this->msgId) += 1;
  }

  void setPPM(uint16_t ppm) {
    *(this->ppm) = ppm;
  }

  uint8_t* getData() {
    return this->data;
  }

  uint8_t getPulserId() {
    return *(this->pulserId);
  }
  
  uint16_t getMessageId() {
    return *(this->msgId);
  }

  uint16_t getPPM() {
    return *(this->ppm);
  }
  
};


#endif
