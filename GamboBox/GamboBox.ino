#include <Adafruit_NeoPixel.h>

#define PIN_LEDS 8
#define NUM_PIXELS 3
#define MAX_BRIGHTNESS 255

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, PIN_LEDS);

uint8_t i;

int* h;

void setup() {

  h = new int[3];
  h[0] = 120;
  h[1] = 120;
  h[2] = 120;

  pixels.begin();
  pixels.setPixelColor(1, 0);
  showPixelsHSV();
}
  
void loop() {

  uint8_t i1 = 1;
  uint8_t i0 = (i1 + random(1, 3)) % 3;
  uint8_t i2 = 3 - i0 - i2;
  
  uint16_t nIterations = random(120, 240);

  for (uint16_t n = 0; n < nIterations; n++) {
      h[i0] = (h[i0] + 1) % 360;
    //  h[i1] = (h[i1] + 1) % 360;
      showPixelsHSV();
  }
  
  for (uint16_t n = 0; n < nIterations; n++) {
      h[i2] = (h[i2] + 1) % 360;
   //   h[i1] = (h[i1] + 1) % 360;
      showPixelsHSV();
  }
 
}


void showPixelsHSV() {

  uint32_t color = 0;

  HSV_to_RGB(h[0], 100.0, 100.0, ((byte*)&color)+2, ((byte*)&color)+1, ((byte*)&color));
  pixels.setPixelColor(0, color);
  
  //HSV_to_RGB(h[1], 100.0, 100.0, ((byte*)&color)+2, ((byte*)&color)+1, ((byte*)&color));
  //pixels.setPixelColor(1, 0);//color);
  
  HSV_to_RGB(h[2], 100.0, 100.0, ((byte*)&color)+2, ((byte*)&color)+1, ((byte*)&color));
  pixels.setPixelColor(2, color);
  pixels.show();
  delay(150);
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

