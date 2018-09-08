#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN 6

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)

Adafruit_NeoPixel strip = Adafruit_NeoPixel(24, PIN, NEO_GRB + NEO_KHZ800);


void setup() {
  Serial.begin(9600);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  for (int i = 24; i > 0; i --) {
    strip.setPixelColor(i, 0, 0, 255);
    strip.show();
    delay(50);
  }
  //Flash strip  Isn't there a way to blank in one command?
  for (int i = 0; i < 6; i++) {
    for (int i = 0; i < 24; i++) {
      strip.setPixelColor(i, 0, 0, 0);
      strip.show();
    }
    delay(200);
    for (int i = 0; i < 24; i ++) {
      strip.setPixelColor(i, 0, 75, 0);
      strip.show();
    }
    delay(200);
  }
}
