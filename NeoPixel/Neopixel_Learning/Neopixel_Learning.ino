#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN 6

int ANIMATION_STEP_DELAY = 10;
int UPDATE_STEP_DELAY = 2000;
int numLitPixels = 1;
bool isIdle = true;
unsigned long startMillis = 0;
unsigned long tickMillis = 0;


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
  startMillis = millis();

}

void loop() {

  if (millis() > (tickMillis + UPDATE_STEP_DELAY)) {
    strip.clear();
    strip.show();
    tickMillis = millis();
    isIdle = false;
  }

  if (!isIdle) {
    updateCountdown(ANIMATION_STEP_DELAY);
  }


  if (numLitPixels > 23) {
    numLitPixels = 0;
    
    isIdle = true;
  }
  //  delay(1000);


  //  for (int i = 24; i > 0; i --) {
  //    strip.setPixelColor(i, 0, 0, 20);
  //    strip.show();
  //    delay(10);
  //  }
  //  //Flash strip
  //  for (int i = 0; i < 2; i++) {
  //    for (int i = 0; i < 24; i++) {
  //      strip.clear();
  //      strip.show();
  //    }
  //
  //    delay(1);
  //
  //    for (int i = 0; i < 24; i ++) {
  //      strip.setPixelColor(i, 0, 10, 0);
  //      strip.show();
  //    }
  //
  //    delay(100);
  //  }
}
