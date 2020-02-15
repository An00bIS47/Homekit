/*
      A benchmark Test, on Teensy it will also check if pin you choose are legal
	  Version 1.11 (better screenFill test, test in rotation 0...3, fixed test lines results)
*/

#include <SPI.h>
#include <SSD_13XX.h>
#include "_icons/wifi.c"

#if defined(__SAM3X8E__)
#undef __FlashStringHelper::F(string_literal)
#define F(string_literal) string_literal
#endif


uint8_t errorCode = 0;
#define _dlyBetweenTests	500

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
#if defined(ESP8266)
#define __CS1   4  //GPIO4 or GPIO2
#define __DC    5   //(D1)
#else
#define __CS1 	10
#define __DC 	9
#endif

SSD_13XX tft = SSD_13XX(__CS1, __DC);

void setup() {
  Serial.begin(38400);
  long unsigned debug_start = millis ();
  while (!Serial && ((millis () - debug_start) <= 5000)) ;
  tft.begin();
  tft.setBrightness(15);

  //the following it's mainly for Teensy
  //it will help you to understand if you have choosed the
  //wrong combination of pins!
  errorCode = tft.getErrorCode();
  if (errorCode != 0) {
    Serial.print("Init error! ");
    if (bitRead(errorCode, 0)) Serial.print("MOSI or SCLK pin mismach!\n");
    if (bitRead(errorCode, 1)) Serial.print("CS or DC pin mismach!\n");
  } else {
    Serial.println(F("Benchmark Sketch V1.11"));
  }
}

void loop(void) {
  for (uint8_t rotation = 0; rotation < 4; rotation++) {
    tft.clearScreen();
    tft.setRotation(rotation);
    Serial.print(F("\nBenchmark[rot="));
    Serial.print(rotation);
    Serial.println(F("]         Time (microseconds)"));
    if (errorCode == 0) {
      Serial.print(F("Screen fill              "));
      Serial.println(testFillScreen());
      delay(_dlyBetweenTests);

      Serial.print(F("Text                     "));
      Serial.println(testText());
      delay(_dlyBetweenTests);

      Serial.print(F("Text2                    "));
      Serial.println(testText2());
      delay(_dlyBetweenTests);

      Serial.print(F("Lines                    "));
      Serial.println(testLines(CYAN));
      delay(_dlyBetweenTests);

      Serial.print(F("Horiz/Vert Lines         "));
      Serial.println(testFastLines(RED, BLUE));
      delay(_dlyBetweenTests);

      Serial.print(F("Arc                      "));
      Serial.println(testArc(CYAN));
      delay(_dlyBetweenTests);

      Serial.print(F("Rectangles (outline)     "));
      Serial.println(testRects(GREEN));
      delay(_dlyBetweenTests);

      Serial.print(F("Rectangles (filled)      "));
      Serial.println(testFilledRects(YELLOW, MAGENTA));
      delay(_dlyBetweenTests);

      Serial.print(F("Circles (filled)         "));
	  #if (SSD_HEIGHT > 64)
      Serial.println(testFilledCircles(10, MAGENTA));
	  #else
	  Serial.println(testFilledCircles(5, MAGENTA));
	  #endif

      Serial.print(F("Circles (outline)        "));
	  #if (SSD_HEIGHT > 64)
      Serial.println(testCircles(10, WHITE));
	  #else
	  Serial.println(testCircles(5, WHITE));
	  #endif
      delay(_dlyBetweenTests);

      Serial.print(F("Triangles (outline)      "));
      Serial.println(testTriangles());
      delay(_dlyBetweenTests);

      Serial.print(F("Triangles (filled)       "));
      Serial.println(testFilledTriangles());
      delay(_dlyBetweenTests);

      Serial.print(F("Rounded rects (outline)  "));
      Serial.println(testRoundRects());
      delay(_dlyBetweenTests);

      Serial.print(F("Rounded rects (filled)   "));
      Serial.println(testFilledRoundRects());
      delay(_dlyBetweenTests);

	  Serial.print(F("Icon Render              "));
	  Serial.println(testIcon());
	  delay(_dlyBetweenTests);

      Serial.println(F("Done! ------------------------"));
    }
    delay(_dlyBetweenTests);
  }
}

unsigned long testIcon() {
	tft.clearScreen();
	unsigned long start = micros();
	tft.drawIcon(0, 0, &wifi);
	return micros() - start;
}

unsigned long testFillScreen() {
  tft.clearScreen();
  unsigned long start = micros();
  tft.fillScreen(RED);
  return micros() - start;
}

unsigned long testText() {
  tft.clearScreen();
  unsigned long start = micros();
  tft.setCursor(0, 0);
  tft.setTextColor(WHITE);
  tft.setTextScale(1);
  tft.println("Hello World!");
  tft.setTextColor(YELLOW);
  tft.setTextScale(2);
  #if (SSD_HEIGHT > 64)
  tft.println(1234.56);
  #else
  tft.println(1234.5);
  #endif
  tft.setTextColor(RED);
  #if (SSD_HEIGHT > 64)
  tft.setTextScale(3);
  #else
  tft.setTextScale(2);
  #endif
  tft.println(0xDEAD, HEX);
  #if (SSD_HEIGHT > 64)
  tft.println();
  tft.setTextColor(GREEN);
  tft.setTextScale(4);
  tft.println("Hello");
  #endif
  return micros() - start;
}

unsigned long testText2() {
  tft.clearScreen();
  unsigned long start = micros();
  tft.setCursor(0, 0);
  tft.setTextColor(WHITE);
  tft.setTextScale(2);
  tft.println("I implore thee,");
  tft.setTextScale(1);
  tft.println("my foonting turlingdromes.");
  tft.println("And hooptiously drangle me");
  tft.println("with crinkly bindlewurdles,");
  tft.println("Or I will rend thee");
  tft.println("in the gobberwarts");
  tft.println("with my blurglecruncheon,");
  tft.println("see if I don't!");
  return micros() - start;
}

unsigned long testLines(uint16_t color) {
  unsigned long start, t;
  int           x1, y1, x2, y2,
                w = tft.width(),
                h = tft.height();

  tft.clearScreen();

  x1 = y1 = 0;
  y2 = h - 1;
  start = micros();
  for (x2 = 0; x2 < w; x2 += 6) tft.drawLine(x1, y1, x2, y2, color);
  x2    = w - 1;
  for (y2 = 0; y2 < h; y2 += 6) tft.drawLine(x1, y1, x2, y2, color);
  t     = micros() - start; // fillScreen doesn't count against timing

  tft.clearScreen();

  x1    = w - 1;
  y1    = 0;
  y2    = h - 1;
  start = micros();
  for (x2 = 0; x2 < w; x2 += 6) tft.drawLine(x1, y1, x2, y2, color);
  x2    = 0;
  for (y2 = 0; y2 < h; y2 += 6) tft.drawLine(x1, y1, x2, y2, color);
  t    += micros() - start;

  tft.clearScreen();

  x1    = 0;
  y1    = h - 1;
  y2    = 0;
  start = micros();
  for (x2 = 0; x2 < w; x2 += 6) tft.drawLine(x1, y1, x2, y2, color);
  x2    = w - 1;
  for (y2 = 0; y2 < h; y2 += 6) tft.drawLine(x1, y1, x2, y2, color);
  t    += micros() - start;

  tft.clearScreen();

  x1    = w - 1;
  y1    = h - 1;
  y2    = 0;
  start = micros();
  for (x2 = 0; x2 < w; x2 += 6) tft.drawLine(x1, y1, x2, y2, color);
  x2    = 0;
  for (y2 = 0; y2 < h; y2 += 6) tft.drawLine(x1, y1, x2, y2, color);
  t    += micros() - start;

  return micros() - start;
}

unsigned long testFastLines(uint16_t color1, uint16_t color2) {
  unsigned long start;
  int           x, y, w = tft.width(), h = tft.height();

  tft.clearScreen();
  start = micros();
  for (y = 0; y < h; y += 5) tft.drawFastHLine(0, y, w, color1);
  for (x = 0; x < w; x += 5) tft.drawFastVLine(x, 0, h, color2);

  return micros() - start;
}


unsigned long testArc(uint16_t color) {
  unsigned long start;
  uint16_t      i,
                cx = tft.width()  / 2,
                cy = tft.height() / 2;

  tft.clearScreen();

  start = micros();
  for (i = 0; i < 360; i += 5) {
    tft.drawArc(cx, cy, 30, 2, 0, i, color);
  }
  return micros() - start;
}


unsigned long testRects(uint16_t color) {
  unsigned long start;
  int           n, i, i2,
                cx = tft.width()  / 2,
                cy = tft.height() / 2;

  tft.clearScreen();
  n     = min(tft.width(), tft.height());
  start = micros();
  for (i = 2; i < n; i += 6) {
    i2 = i / 2;
    tft.drawRect(cx - i2, cy - i2, i, i, color);
  }

  return micros() - start;
}

unsigned long testFilledRects(uint16_t color1, uint16_t color2) {
  unsigned long start, t = 0;
  int           n, i, i2,
                cx = (tft.width()  / 2) - 1,
                cy = (tft.height() / 2) - 1;

  tft.clearScreen();
  n = min(tft.width(), tft.height());
  for (i = n; i > 0; i -= 6) {
    i2    = i / 2;
    start = micros();
    tft.fillRect(cx - i2, cy - i2, i, i, color1);
    t    += micros() - start;
    // Outlines are not included in timing results
    tft.drawRect(cx - i2, cy - i2, i, i, color2);
  }

  return t;
}

unsigned long testFilledCircles(uint8_t radius, uint16_t color) {
  unsigned long start;
  int x, y, w = tft.width(), h = tft.height(), r2 = radius * 2;

  tft.clearScreen();
  start = micros();
  for (x = radius; x < w; x += r2) {
    for (y = radius; y < h; y += r2) {
      tft.fillCircle(x, y, radius, color);
    }
  }

  return micros() - start;
}

unsigned long testCircles(uint8_t radius, uint16_t color) {
  unsigned long start;
  int           x, y, r2 = radius * 2,
                      w = tft.width()  + radius,
                      h = tft.height() + radius;

  // Screen is not cleared for this one -- this is
  // intentional and does not affect the reported time.
  start = micros();
  for (x = 0; x < w; x += r2) {
    for (y = 0; y < h; y += r2) {
      tft.drawCircle(x, y, radius, color);
    }
  }

  return micros() - start;
}

unsigned long testTriangles() {
  unsigned long start;
  int           n, i, cx = tft.width()  / 2 - 1,
                      cy = (tft.height() / 2) - 1;

  tft.clearScreen();
  n     = min(cx, cy);
  start = micros();
  for (i = 0; i < n; i += 5) {
    tft.drawTriangle(
      cx    , cy - i, // peak
      cx - i, cy + i, // bottom left
      cx + i, cy + i, // bottom right
      tft.Color565(0, 0, i));
  }

  return micros() - start;
}

unsigned long testFilledTriangles() {
  unsigned long start, t = 0;
  int           i, cx = (tft.width() / 2) - 1,
                   cy = tft.height() / 2 - 1;

  tft.clearScreen();
  start = micros();
  for (i = min(cx, cy); i > 10; i -= 5) {
    start = micros();
    tft.fillTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
                     tft.Color565(0, i, i));
    t += micros() - start;
    tft.drawTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
                     tft.Color565(i, i, 0));
  }

  return t;
}

unsigned long testRoundRects() {
  unsigned long start;
  int           w, i, i2,
                cx = (tft.width()  / 2) - 1,
                cy = (tft.height() / 2) - 1;

  tft.clearScreen();
  w     = min(tft.width(), tft.height());
  start = micros();
  for (i = 0; i < w; i += 6) {
    i2 = i / 2;
    tft.drawRoundRect(cx - i2, cy - i2, i, i, i / 8, tft.Color565(i, 0, 0));
  }

  return micros() - start;
}

unsigned long testFilledRoundRects() {
  unsigned long start;
  int           i, i2,
                cx = (tft.width()  / 2) - 1,
                cy = (tft.height() / 2) - 1;

  tft.clearScreen();
  start = micros();
  for (i = min(tft.width(), tft.height()); i > 20; i -= 6) {
    i2 = i / 2;
    tft.fillRoundRect(cx - i2, cy - i2, i, i, i / 8, tft.Color565(0, i, 0));
  }

  return micros() - start;
}

/*
Benchmark[rot=0]         Time (microseconds)
Screen fill              231077
Text                     13780
Text2                    45995
Lines                    391310
Horiz/Vert Lines         110942
Rectangles (outline)     70637
Rectangles (filled)      2399056
Circles (filled)         401570
Circles (outline)        336834
Triangles (outline)      93285
Triangles (filled)       941500
Rounded rects (outline)  139741
Rounded rects (filled)   2741213
Done! ------------------------
*/