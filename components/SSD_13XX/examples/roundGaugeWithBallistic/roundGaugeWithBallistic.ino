/*
ROUND GAUGE EXAMPLE with ballistic!
This example show how to create a round gauge that react like the real one with (almost) correct ballistic
Created by S.U.M.O.T.O.Y - Max MC Costa
If you modify or get better result please let me know
*/
#include <SPI.h>
#include <SSD_13XX.h>



volatile int16_t curVal1 = 0;
volatile int16_t oldVal1 = 0;

/*
Teensy3.x and Arduino's
You are using 4 wire SPI here, so:
 MOSI:  11//Teensy3.x/Arduino UNO (for MEGA/DUE refere to arduino site)
 MISO:  12//Teensy3.x/Arduino UNO (for MEGA/DUE refere to arduino site)
 SCK:   13//Teensy3.x/Arduino UNO (for MEGA/DUE refere to arduino site)
ESP8266-----------------------------------
Use:
#define __CS  16  //(D0)
#define __DC  5   //(D1)
#define __RST 4   //(D2)

 SCLK:D5
 MOSI:D7
 */
#define __CS  10
#define __DC  9

#define _GAUGEDIM 31

/*
Teensy 3.x can use: 2,6,10,15,20,21,22,23
Arduino's 8 bit: any
DUE: check arduino site
If you do not use reset, tie it to +3V3
*/


SSD_13XX tft = SSD_13XX(__CS, __DC);


void setup() {
  Serial.begin(9600);
  tft.begin();
  drawGauge(_GAUGEDIM, _GAUGEDIM, _GAUGEDIM);
}

void loop(void) {
  curVal1 = random(1, 254);
  if (oldVal1 != curVal1) {
    drawNeedle(curVal1, _GAUGEDIM, _GAUGEDIM, _GAUGEDIM, GREEN, BLACK);
    oldVal1 = curVal1;
  }
}

void drawGauge(uint8_t x, uint8_t y, uint8_t r) {
  tft.drawCircle(x, y, r, WHITE); //draw instrument container
  faceHelper(x, y, r, 150, 390, 1.3); //draw major ticks
  if (r > 15) faceHelper(x, y, r, 165, 375, 1.1); //draw minor ticks

}

void faceHelper(uint8_t x, uint8_t y, uint8_t r, int from, int to, float dev) {
  float dsec, fromSecX, fromSecY, toSecX, toSecY;
  int i;
  for (i = from; i <= to; i += 30) {
    dsec = i * (PI / 180);
    fromSecX = cos(dsec) * (r / dev);
    fromSecY = sin(dsec) * (r / dev);
    toSecX = cos(dsec) * r;
    toSecY = sin(dsec) * r;
    tft.drawLine(1 + x + fromSecX, 1 + y + fromSecY, 1 + x + toSecX, 1 + y + toSecY, WHITE);
  }
}

void drawNeedle(int16_t val, uint8_t x, uint8_t y, uint8_t r, uint16_t color, uint16_t bcolor) {
  uint8_t i;
  if (curVal1 > oldVal1) {
    for (i = oldVal1; i <= curVal1; i++) {
      drawPointerHelper(i - 1, _GAUGEDIM, _GAUGEDIM, _GAUGEDIM, bcolor);
      drawPointerHelper(i, _GAUGEDIM, _GAUGEDIM, _GAUGEDIM, color);
      if ((curVal1 - oldVal1) < (128)) delay(1);//ballistic
    }
  }
  else {
    for (i = oldVal1; i >= curVal1; i--) {
      drawPointerHelper(i + 1, _GAUGEDIM, _GAUGEDIM, _GAUGEDIM, bcolor);
      drawPointerHelper(i, _GAUGEDIM, _GAUGEDIM, _GAUGEDIM, color);
      //ballistic
      if ((oldVal1 - curVal1) >= 128) {
        delay(1);
      } else {
        delay(3);
      }
    }
  }
}

void drawPointerHelper(int16_t val, uint8_t x, uint8_t y, uint8_t r, uint16_t color) {
  float dsec, toSecX, toSecY;
  int16_t minValue = 0;
  int16_t maxValue = 255;
  int fromDegree = 150;//start
  int toDegree = 240;//end
  if (val > maxValue) val = maxValue;
  if (val < minValue) val = minValue;
  dsec = (((float)(uint16_t)(val - minValue) / (float)(uint16_t)(maxValue - minValue) * toDegree) + fromDegree) * (PI / 180);
  toSecX = cos(dsec) * (r / 1.35);
  toSecY = sin(dsec) * (r / 1.35);
  tft.drawLine(x, y, 1 + x + toSecX, 1 + y + toSecY, color);
  tft.fillCircle(x, y, 2, color);
}

