#include <Adafruit_NeoPixel.h>

#define PIN_LEDS 0
#define PIN_BUTTON1 1
#define PIN_BUTTON2 2
#define PIN_POTI 4
#define AREAD_POTI 2

#define NUM_PIXELS 42
#define MAX_BRIGHTNESS 200

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, PIN_LEDS);

uint8_t i = 1;
uint8_t j = 0;
uint8_t k = 0;
uint8_t m = 0;
uint8_t incr = 1;
uint8_t mode1 = 2;
uint8_t mode2 = 0;
bool pressedButton1 = false;
bool pressedButton2 = false;

void setup() {
  
  pinMode(PIN_BUTTON1, INPUT);
  pinMode(PIN_BUTTON2, INPUT);
  pinMode(PIN_POTI, INPUT);

  pixels.begin();
  pixels.setBrightness(MAX_BRIGHTNESS);
}
  
void loop() {
  
   if (analogRead(1) < 200) { // PIN_BUTTON2 !!

    if (!pressedButton2) {
      mode2 = (mode2 + 1) % 3;
      pressedButton2 = true;
      i = 1;
      j = 0;
    }

    if (pressedButton1) {
      pressedButton1 = false;
    }
  }
  else if (digitalRead(PIN_BUTTON1) == LOW) {

    if (!pressedButton1) {
      mode1 = (mode1 + 1) % 3;
      pressedButton1 = true;
      k = 0;
      m = 0;
    }

    if (pressedButton2) {
      pressedButton2 = false;
    } 
  }
  else {
      pressedButton1 = false;
      pressedButton2 = false;
  }



  if (mode1 == 0) {
    pixels.setPixelColor(0, 0);
    pixels.setPixelColor(NUM_PIXELS - 1, 0);
  }
  else if (mode1 == 1) {
    flickerStep();
  }
  else {
    pixels.setPixelColor(0, 0xffff00);
    pixels.setPixelColor(NUM_PIXELS - 1, 0xffff00);    
  }


  if (mode2 == 0) {
    for (i = 1; i < NUM_PIXELS-1; i++)
      pixels.setPixelColor(i, 0);
  }
  else if (mode2 == 1) {
    vitalsStep();
  }
  else {
    kittStep();
  }

  
  pixels.setBrightness(20 + (MAX_BRIGHTNESS-20) * (analogRead(AREAD_POTI) / 1023.0));
  
  pixels.show();

  delay(50);
}


void flickerStep() {

  if (m == 0) {
    m = (millis() % 7) + 1;
    k = abs(k - 1);
  }

  pixels.setPixelColor(0, 0xffff00 * k);
  pixels.setPixelColor(NUM_PIXELS - 1, 0xffff00 * k);    

  m--;
}


void vitalsStep() {
  unsigned long t = millis()/100;

  if (sin(t) > 0.2 && sin(t) < 0.6) {
    pixels.setPixelColor(NUM_PIXELS - 4, 0xff0000);
  }
  else {
    pixels.setPixelColor(NUM_PIXELS - 4, 0);
  }

  for (i = 0; i < 8; i++) {

    if ((( j & 0xff ) >> i) & 0x1) {
      pixels.setPixelColor(i*2 + 1, 0xff00);
    }
    else {
      pixels.setPixelColor(i*2 + 1, 0);
    }
  }

  j = (j + 1) % 256;
}


void kittStep() {

  for (j = 1; j < NUM_PIXELS - 1; j++)
    pixels.setPixelColor(j, 0);

  i = (i + incr);

  if (i == NUM_PIXELS-1) {
    incr = -1;
    i = NUM_PIXELS - 2;
  }
  else if (i == 0) {
    incr = 1;
    i = 1;
  }

  if (i > 3)
    pixels.setPixelColor(i-3, 0x1f0000);
  if (i > 2)
    pixels.setPixelColor(i-2, 0x3f0000);
  if (i > 1)
    pixels.setPixelColor(i-1, 0xff0000);
  pixels.setPixelColor(i, 0xff0000);
  if (i < NUM_PIXELS-2)
    pixels.setPixelColor(i+1, 0xff0000);
  if (i < NUM_PIXELS-3)
    pixels.setPixelColor(i+2, 0x3f0000);
  if (i < NUM_PIXELS-4)
    pixels.setPixelColor(i+3, 0x1f0000);
}


