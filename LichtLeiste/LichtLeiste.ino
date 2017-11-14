#include <Adafruit_NeoPixel.h>

#define PIN_LEDS 8
#define NUM_PIXELS 3
#define MAX_BRIGHTNESS 255

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, PIN_LEDS);

uint8_t i;
uint32_t clr = 0xffffff;
uint8_t mode = 0;


uint32_t clr0 = 0x00ff00;
uint32_t clr1 = 0x00ff00;
uint32_t clr2 = 0x00ff00;

void setup() {
  delay(10);
  pixels.begin();
 // pixels.setBrightness(MAX_BRIGHTNESS);
  
  setAllPixels(clr);
  pixels.show();
}
  
void loop() {

  mode = random(0, 3);

  if (mode == 0)
    mode = 1;
  else if (mode == 1)
    mode = 0;

  if (mode == 0) { // change right pixel first

    clr2 = nextColor(clr2, 8);

    clr1 = nextColor(clr1, 4);

    clr0 = nextColor(clr0);

    showPixels();

    while (!isMainColor(clr0)) { 

      if (!isMainColor(clr2))
        clr2 = nextColor(clr2, 8);

      if (!isMainColor(clr1))
        clr1 = nextColor(clr1, 4);
  
      clr0 = nextColor(clr0);
  
      showPixels();
    } 
  }


 /* 
  nextColorGlob();
  setAllPixels(clr);     
  pixels.show();
  delay(50);
*/
}

void showPixels() {
  pixels.setPixelColor(0, clr0);
  pixels.setPixelColor(1, clr1);
  pixels.setPixelColor(2, clr2);
  pixels.show();
  delay(20);
}


void setAllPixels(uint32_t color) {
  for (i = 0; i < NUM_PIXELS; i++)
    pixels.setPixelColor(i, color);
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

