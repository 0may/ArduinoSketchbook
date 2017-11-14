#include <Adafruit_NeoPixel.h>

#define PIN_LEDS 1
#define NUM_PIXELS 25
#define MAX_BRIGHTNESS 200

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, PIN_LEDS);

uint8_t i;
uint32_t clr = 0xffff00;

float scale[NUM_PIXELS];


void setup() {
  
  pixels.begin();
  pixels.setBrightness(MAX_BRIGHTNESS);

  setPixelsScaled(clr);
  pixels.show();
}
  
void loop() {

  nextColorGlob();

  setScale();
  
  setPixelsScaled(clr);
    
  pixels.show();
  
  delay(40);
}

void setPixelsScaled(uint32_t color) {
  uint32_t c;
  
  for (i = 0; i < NUM_PIXELS; i++) {

    c = ((uint32_t)(((color >> 16) & 0xff) * scale[i]) << 16) + ((uint32_t)(((color >> 8) & 0xff) * scale[i]) << 8) + (uint32_t)((color & 0xff) * scale[i]);
    
    pixels.setPixelColor(i, c);
  }
}

void setScale() {
  unsigned long t = millis();
  
  for (i = 0; i < NUM_PIXELS; i++) {
    scale[i] = 0.5 + 0.5*sin((i + t/2000.0));
    scale[i] *= scale[i];
  }
  
}

void setAllPixels(uint32_t color) {
  for (i = 0; i < NUM_PIXELS; i++)
    pixels.setPixelColor(i, (uint32_t)(color * scale[i]));
}



void nextColorGlob() {
  
  if ((clr & 0x0000ff) == 255
      && (clr & 0x00ff00) > 0) 
      clr = clr - 0x000100;
  else if ((clr & 0x0000ff) == 255
      && ((clr & 0xff0000) >> 16) < 255) 
      clr = clr + 0x010000;
  else if (((clr & 0xff0000) >> 16) == 255
      && (clr & 0x0000ff) > 0) 
      clr =  clr - 0x000001;
  else if (((clr & 0xff0000) >> 16) == 255
      && ((clr & 0x00ff00) >> 8) < 255) 
      clr = clr + 0x000100;
  else if (((clr & 0x00ff00) >> 8) == 255
      && (clr & 0xff0000) > 0) 
      clr = clr - 0x010000;
  else if (((clr & 0x00ff00) >> 8) == 255
      && ((clr & 0x0000ff)) < 255) 
      clr = clr + 0x000001;     
  else 
      clr = 0xff;
}


uint32_t nextColor(uint32_t color) {
  
  if ((color & 0x0000ff) == 255
      && (color & 0x00ff00) > 0) 
      color = color - 0x000100;
  else if ((color & 0x0000ff) == 255
      && ((color & 0xff0000) >> 16) < 255) 
      color = color + 0x010000;
  else if (((color & 0xff0000) >> 16) == 255
      && (color & 0x0000ff) > 0) 
      color =  color - 0x000001;
  else if (((color & 0xff0000) >> 16) == 255
      && ((color & 0x00ff00) >> 8) < 255) 
      color = color + 0x000100;
  else if (((color & 0x00ff00) >> 8) == 255
      && (color & 0xff0000) > 0) 
      color = color - 0x010000;
  else if (((color & 0x00ff00) >> 8) == 255
      && ((color & 0x0000ff)) < 255) 
      color = color + 0x000001;     
  else 
      color = 0xff;

  return color;
}

uint32_t nextColor(uint32_t color, uint8_t incr) {

  for (uint8_t j = 0; j < incr; j++) {
    
    if ((color & 0x0000ff) == 255
        && (color & 0x00ff00) > 0) 
        color = color - 0x000100;
    else if ((color & 0x0000ff) == 255
        && ((color & 0xff0000) >> 16) < 255) 
        color = color + 0x010000;
    else if (((color & 0xff0000) >> 16) == 255
        && (color & 0x0000ff) > 0) 
        color =  color - 0x000001;
    else if (((color & 0xff0000) >> 16) == 255
        && ((color & 0x00ff00) >> 8) < 255) 
        color = color + 0x000100;
    else if (((color & 0x00ff00) >> 8) == 255
        && (color & 0xff0000) > 0) 
        color = color - 0x010000;
    else if (((color & 0x00ff00) >> 8) == 255
        && ((color & 0x0000ff)) < 255) 
        color = color + 0x000001;     
    else 
        color = 0xff;
  }

  return color;
}


bool isMainColor(uint32_t color) {

  return (color == 0x0000ff 
       || color == 0x00ff00 
       || color == 0xff0000 
       || color == 0x00ffff 
       || color == 0xff00ff 
       || color == 0xffff00);
}

