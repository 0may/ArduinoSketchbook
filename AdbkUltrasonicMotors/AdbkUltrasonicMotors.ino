/*
 * Software License Agreement (BSD License)
 *
 * Copyright (c) 2017 Oliver Mayer, Akademie der Bildenden Kuenste Nuernberg. 
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
 * Steuerung von Motoren über Entfernungsmessung durch Ultraschallsensoren.
 *****************************************************************************/

#include <RunningMedian.h>
#include <Ultrasonic.h>

uint8_t numModules = 2;  // Anzahl der verwendeten Ultraschallsensor-Motor Paare (Module). Darf nicht größer als 5 sein


// Jedes Modul (Paar aus Ultraschallsensor und Motor) muss die unten beschriebenen Pins verwenden, z.B. 
// das erste Modul verbindet Arduino Pin 2 mit dem SIG Pin am Ultraschallsensor, Arduino Pin 12 mit dem 
// DIR Pin der Motorsteuerung und Arduino Pin 3 mit dem PWM Pin der Motorsteuerung. Jedes weitere Modul
// verwendet entsprechend die Pins, die in den nächsten Zeilen untereinander stehen
uint8_t uSensorPins[] =  { 2,  4,  6,  7,  8};        // Ultraschallsensor Pins
uint8_t motorDirPins[] = {12, 13, A3, A4, A5};  // Pins für Motorrichtung (DIR oder PHASE Pin an der Motorsteuerung)
uint8_t motorPwmPins[] = { 3,  5,  9, 10, 11};     // Pins für Motorgeschwindigkeit (PWM oder ENABLE Pin an der Motorsteuerung)


Ultrasonic** uSensors;                   // Liste von Ultraschallsensor Objekten
RunningMedian** uSensorMedianDistances;  // Liste von Objekten, die den Median der letzten <medianWindowSize> Distanz-Messungen berechnen

uint8_t medianWindowSize = 5;   // Anzahl der letzten Sensorwerte, die in die Medianberechnung einfliessen
float distanceThreshold = 0.0;  // Distanz-Schwellwert (0-520): Sind gemessene Distanzen kleiner als dieser Wert, werden die Motoren bewegt. Wird mit Poti an A0 gesteuert.
uint8_t motorSpeed = 0;         // Motor-Geschwindigkeit (0-255). Wird mit Poti an A2 gesteuert.
long motorSpinMillisMax = 1023; // Maximale Anzahl an Millisekunden, die ein Motor vorwärts drehen darf (0-1023). Wird mit Poti an A1 gesteuert.
long* motorSpinMillis;          // Array mit aktueller Anzahl an Millisekunden, die ein Motor vorwärts gedreht wurde. Rückwärts drehen verringert diesen Wert.
unsigned long* lastMillis;      // Array des letzten Zeitpunkts, an dem ein Motor geupdated wurde (updateMotors())
int8_t* motorSpinDir;           // Array der aktuellen Drehrichtung (vorwärts=1, rückwärts=-1, gebremst=0) jedes Motors

bool manualMotorMode = false;   // Modus, um Motoren manuell zu steuern
uint8_t manualMotorIdx = 0;     // Index des Motors, der aktuell gesteuert werden soll


// Die setup() Funktion wird einmal nach Start des Arduino aufgerufen.
// Hier werden Werte und Objekte initialisiert.
void setup()
{
  Serial.begin(9600);

  // Prüfe die Anzahl Module und setze auf maximal 5
  if (numModules > 5) 
    numModules = 5;

  // Initialisiere Arrays
  uSensors = new Ultrasonic*[numModules];
  uSensorMedianDistances = new RunningMedian*[numModules];
  motorSpinMillis = new long[numModules];
  lastMillis = new unsigned long[numModules];
  motorSpinDir = new int8_t[numModules];

  // Initialisiere Array-Elemente und Ausgabe-Pins
  for (uint8_t n = 0; n < numModules; n++) 
  {
    uSensors[n] = new Ultrasonic(uSensorPins[n]);
    uSensorMedianDistances[n] = new RunningMedian(medianWindowSize);
    motorSpinMillis[n] = 0;
    lastMillis[n] = 0;
    motorSpinDir[n] = 0;

    pinMode(motorDirPins[n], OUTPUT);
    pinMode(motorPwmPins[n], OUTPUT);
    motorBrake(n);
  }   

  // Wenn der Regler an A0 am rechten Ende steht, wird der Modus zur manuellen Motorsteuerung gestartet.
  // Im manuellen Modus können Motoren direkt mit den drei Reglern gesteuert werden:
  // - Regler 1: Auswahl des Motors
  // - Regler 2: Vorwärts-Drehen des ausgewählten Motors
  // - Regler 3: Wenn Regler 2 ganz links steht, dann Rückwärts-Drehen des ausgewählten Motors
  if (analogRead(A0) == 1023)
    manualMotorMode = true;
}


void loop()
{
  if (!manualMotorMode) 
  {
    updateDistances(false);
  
    updateControls(false);
  
    updateMotors(false);
  }
  else 
  {
    updateManualMotorsMode(false);   
  }
  
  delay(10);
}


// misst Distanzen mit den einzelnen Ultraschallsensoren
void updateDistances(bool printToSerial)
{
    for (uint8_t n = 0; n < numModules; n++) 
    {
      uSensorMedianDistances[n]->add(uSensors[n]->MeasureInCentimeters());
      
      if (printToSerial) 
      {
        Serial.print("S"); 
        Serial.print(n);
        Serial.println((int)(uSensorMedianDistances[n]->getMedian()));
      }
    
      //delay(1);
    }
}


// liest die regelbaren Werte neu ein
void updateControls(bool printToSerial) 
{
  distanceThreshold = analogRead(0)/1023.0 * 520.0;
  motorSpinMillisMax = analogRead(1);
  motorSpeed = analogRead(2) >> 2;

  if (printToSerial) 
  {
    Serial.print("thresh=");
    Serial.print(distanceThreshold);
    Serial.print(" msmax=");
    Serial.print(motorSpinMillisMax);
    Serial.print(" speed=");
    Serial.println(motorSpeed);
  }
}


// steuert die Motoren an entsprechend der gemessenen Distanzen und 
// eingestellten Regler-Werte
void updateMotors(bool printToSerial) 
{
  unsigned long currentMillis;
  long targetSpinMillis;
  int currentDist;

  for (uint8_t n = 0; n < numModules; n++) {
    currentDist = (int)uSensorMedianDistances[n]->getMedian();
    currentMillis = millis();

    if (motorSpinDir[n] == 1) 
    {
      motorSpinMillis[n] += (long)(currentMillis - lastMillis[n]);
    }
    else if (motorSpinDir[n] == -1) 
    {
      motorSpinMillis[n] -= (long)(currentMillis - lastMillis[n]);
    }
  
    if (currentDist < distanceThreshold) 
    {
      targetSpinMillis = (long)(motorSpinMillisMax*(1.0-currentDist/distanceThreshold) + 0.5);
    }
    else 
    {
      targetSpinMillis = 0;
    }

    if (motorSpinMillis[n] < targetSpinMillis - 10*numModules) 
    {
      motorSpinForward(n);
      motorSpinDir[n] = 1;
      lastMillis[n] = currentMillis;     
    }
    else if (motorSpinMillis[n] > targetSpinMillis + 10*numModules) 
    {
      motorSpinBackward(n);
      motorSpinDir[n] = -1;
      lastMillis[n] = currentMillis;
    }
    else 
    {
      motorBrake(n);
      motorSpinDir[n] = 0;
      lastMillis[n] = currentMillis ;   
    }

    if (printToSerial) 
    {
      Serial.print("M"); Serial.print(n); 
      Serial.print(": dir="); Serial.print(motorSpinDir[n]);
      Serial.print(" ms="); Serial.println(motorSpinMillis[n]);
    }
  }
}


// steuert die Motoren manuell über die drei Regler
void updateManualMotorsMode(bool printToSerial) 
{
  uint8_t idx = (uint8_t)(analogRead(A0)/1024.0 * numModules);

  if (idx != manualMotorIdx) 
  {
    motorBrake(manualMotorIdx);
    manualMotorIdx = idx;
  }

  motorSpeed = analogRead(A1) >> 2;
  
  if (motorSpeed > 0) 
  {
    motorSpinForward(manualMotorIdx);

    if (printToSerial) {
      Serial.print("M"); Serial.print(manualMotorIdx); 
      Serial.print(": dir="); Serial.print(1);
      Serial.print(" speed="); Serial.println(motorSpeed);        
    }
  }
  else 
  {
    motorSpeed = analogRead(A2) >> 2;

    if (motorSpeed > 0) 
    {
      motorSpinBackward(manualMotorIdx);

      if (printToSerial) {
        Serial.print("M"); Serial.print(manualMotorIdx); 
        Serial.print(": dir="); Serial.print(-1);
        Serial.print(" speed="); Serial.println(motorSpeed);        
      }
    }
    else 
    {
      motorBrake(manualMotorIdx);

      if (printToSerial) {
        Serial.print("M"); Serial.print(manualMotorIdx); 
        Serial.print(": dir="); Serial.println(0);     
      }
    }
  }
}


void motorSpinForward(uint8_t n) 
{
  if (n < numModules) 
  {
    digitalWrite(motorDirPins[n], HIGH);
    analogWrite(motorPwmPins[n], motorSpeed);
  }
}


void motorSpinBackward(uint8_t n) 
{
  if (n < numModules) 
  {
    digitalWrite(motorDirPins[n], LOW);
    analogWrite(motorPwmPins[n], motorSpeed);
  }
}


void motorBrake(uint8_t n) 
{
  if (n < numModules) 
  {
    digitalWrite(motorPwmPins[n], LOW);
  }
}

