#include <Adafruit_NeoPixel.h>

#define LED_STRIP 5
#define LED_EYE 11

#define LED_BOARD_A_G 12
#define LED_BOARD_A_R 10
#define LED_BOARD_B_G 9

#define BUTTON_A 8
#define BUTTON_B 4
#define POTI_A A1
#define POTI_B A5

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(7, LED_STRIP);

bool pressedButtonA = false;
bool pressedButtonB = false;

uint8_t mode_jewel = 0;
uint8_t mode_eye = 0;
uint8_t mode_boards = 0;
float freq_boards = 0.0;

uint8_t flickerCnt_AG = 0;
uint8_t flickerTgt_AG = 0;
boolean flickerState_AG = false;

uint8_t flickerCnt_AR = 0;
uint8_t flickerTgt_AR = 0;
boolean flickerState_AR = false;

uint8_t flickerCnt_BG = 0;
uint8_t flickerTgt_BG = 0;
boolean flickerState_BG = false;

double strip_current = 0.0;
double strip_target = 1.0;

float strip_state[9];

float jfreq;

uint8_t i;
unsigned long t; 

void setup() {

  pinMode(LED_EYE, OUTPUT);
  pinMode(LED_BOARD_A_G, OUTPUT);
  pinMode(LED_BOARD_A_R, OUTPUT);
  pinMode(LED_BOARD_B_G, OUTPUT);
  
  pinMode(BUTTON_A, INPUT);
  pinMode(BUTTON_B, INPUT);
  pinMode(POTI_A, INPUT);
  pinMode(POTI_B, INPUT);

  pixels.begin();
  pixels.setBrightness(255);

  for (i = 0; i < 9; i++) {
    if (random(10) < 2) 
      strip_state[i] = 1.0;
    else 
      strip_state[i] = 0.0;
  }
}
  
void loop() {


  float valPotiA = analogRead(POTI_A)/1023.0;
  float valPotiB = analogRead(POTI_B)/1023.0;

  t = millis();
  
  if (digitalRead(BUTTON_B) == LOW) {

    if (!pressedButtonB) {

      pressedButtonB = true;
      mode_eye = (mode_eye + 1) % 3;

      if (mode_eye == 0) {
        digitalWrite(LED_EYE, LOW);
      }
      else if (mode_eye == 2) {
        analogWrite(LED_EYE, 255*valPotiB);
      }

      if (pressedButtonA) {
        mode_jewel = (mode_jewel + 1) % 2;
      }
    }

   // if (pressedButtonA) {
   //   pressedButtonA = false;
   // } 
  }
  else {
    pressedButtonB = false;
  }
 // else 
  
  if (digitalRead(BUTTON_A) == LOW) {

    if (!pressedButtonA) {
      pressedButtonA = true;
      
      mode_boards = (mode_boards + 1) % 3;

      if (mode_boards == 0) {
        digitalWrite(LED_BOARD_A_G, LOW);
        digitalWrite(LED_BOARD_A_R, LOW);
        digitalWrite(LED_BOARD_B_G, LOW);
      }
      else if (mode_boards == 2) {
        digitalWrite(LED_BOARD_A_G, HIGH);
        digitalWrite(LED_BOARD_A_R, HIGH);
        digitalWrite(LED_BOARD_B_G, HIGH);
      }

      if (pressedButtonB) {
        mode_jewel = (mode_jewel + 1) % 2;
      }
    }

  //  if (pressedButtonB) {
  //    pressedButtonB = false;
  //  } 
  }
  else {
    pressedButtonA = false;
  }
  //else {
  //    pressedButtonA = false;
  //    pressedButtonB = false;
  //}


  if (mode_eye == 1) {
    float a = (0.5 + 0.5*sin(t*valPotiB*0.01));
    a *= a;
    analogWrite(LED_EYE, a*255);
  }


  if (mode_boards == 1) {
    freq_boards = valPotiA;

    flickerBoards();
  }

  jfreq = 500.0/(valPotiB + 1.0);
  updateJewel();

  delay(5);
}

void flickerBoards() {

  if (flickerCnt_AG == flickerTgt_AG) {
    flickerCnt_AG = 0;
    flickerState_AG = !flickerState_AG;
    
    if (flickerState_AG) {
      flickerTgt_AG = random(3, 20 - (freq_boards*10));
      digitalWrite(LED_BOARD_A_G, HIGH);
    }
    else {
      if (random(100) <= 50*(1.0-freq_boards))
        flickerTgt_AG = random(5 + (1.0-freq_boards)*100, 105 + (1.0-freq_boards)*300);
      else
        flickerTgt_AG = random(3, 100 - (freq_boards*90));
      digitalWrite(LED_BOARD_A_G, LOW);
    }
  }
  
  if (flickerCnt_AR == flickerTgt_AR) {
    flickerCnt_AR = 0;
    flickerState_AR = !flickerState_AR;
    
    if (flickerState_AR) {
      flickerTgt_AR = random(3, 20 - (freq_boards*10));
      digitalWrite(LED_BOARD_A_R, HIGH);
    }
    else {
      if (random(100) <= 50*(1.0-freq_boards))
        flickerTgt_AR = random(5 + (1.0-freq_boards)*100, 105 + (1.0-freq_boards)*300);
      else
        flickerTgt_AR = random(3, 100 - (freq_boards*90));
      digitalWrite(LED_BOARD_A_R, LOW);
    }
  }

  if (flickerCnt_BG == flickerTgt_BG) {
    flickerCnt_BG = 0;
    flickerState_BG = !flickerState_BG;
    
    if (flickerState_BG) {
      flickerTgt_BG = random(3, 20 - (freq_boards*10));
      digitalWrite(LED_BOARD_B_G, HIGH);
    }
    else {
      if (random(100) <= 50*(1.0-freq_boards))
        flickerTgt_BG = random(5 + (1.0-freq_boards)*100, 105 + (1.0-freq_boards)*300);
      else
        flickerTgt_BG = random(3, 100 - (freq_boards*90));
      digitalWrite(LED_BOARD_B_G, LOW);
    }
  }

  flickerCnt_AG++;
  flickerCnt_AR++;
  flickerCnt_BG++;
  
}



void updateJewel() {
/*
  float incr = 0.01;

  for (i = 0; i < 9; i++) {

    if (strip_state[i] == 1.0) {

      if (i > 1 && strip_state[i-1] == 0.0 && random(10) < 3) {
        strip_state[i-1] += incr;
      }

      if (i < 8 && strip_state[i+1] == 0.0 && random(10) < 3) {
        strip_state[i+1] += incr;
      }   

      strip_state[i] = -1.0;
    }  
  }

  for (i = 0; i < 9; i++) {

    if (strip_state[i] == 0.0) {
      if (random(10) < 1) 
        strip_state[i] += incr;
    }
    else {
      strip_state[i] += incr;
    }

    pixels.setPixelColor(i, 0, 255*abs(strip_state[i]), 0);
  }
*/
  float v;
  uint8_t c = 10;
  float f = jfreq;//200.0;
  
  pixels.setPixelColor(0, (uint32_t)((c-5) * (0.5 + 0.5*sin(t/500.0)) + 0.5) << 8);

  v = 0.5 + 0.5*sin(1 + t/f);
  v *= v*v*v*v*v*v;
  pixels.setPixelColor(1, (uint32_t)(c * v + 0.5) << 16);

  v = 0.5 + 0.5*sin(2 + t/f);
  v *= v*v*v*v*v*v;  
  pixels.setPixelColor(2, (uint32_t)(c * v + 0.5) << 16);

  v = 0.5 + 0.5*sin(3 + t/f);
  v *= v*v*v*v*v*v;  
  pixels.setPixelColor(3, (uint32_t)(c * v + 0.5) << 16);

  v = 0.5 + 0.5*sin(4 + t/f);
  v *= v*v*v*v*v*v;  
  pixels.setPixelColor(4, (uint32_t)(c * v + 0.5) << 16);

  v = 0.5 + 0.5*sin(5 + t/f);
  v *= v*v*v*v*v*v;  
  pixels.setPixelColor(5, (uint32_t)(c * v + 0.5) << 16);

  v = 0.5 + 0.5*sin(6 + t/f);
  v *= v*v*v*v*v*v;  
  pixels.setPixelColor(6, (uint32_t)(c * v + 0.5) << 16);

  if (mode_jewel == 0) {
    pixels.setPixelColor(0, 0);
    pixels.setPixelColor(1, 0);
    pixels.setPixelColor(2, 0);
    pixels.setPixelColor(3, 0);
    pixels.setPixelColor(4, 0);
    pixels.setPixelColor(5, 0);
    pixels.setPixelColor(6, 0);
  }
  
  pixels.show();
  
}


