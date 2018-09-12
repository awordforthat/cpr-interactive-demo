#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN 6

int ANIMATION_STEP_DELAY = 15;
int UPDATE_STEP_DELAY = 500; //Time before we update the stick.  To be replaced by a call to update
//unsigned long tickMillis = 0;
int NUM_PIXELS = 24;

bool isIdle = true;
bool decrementNow = false;
unsigned long startMillis = 0;
int numLitPixels = 1; //Number of pixels to be lit upon updating the stick
int lastLitPixel = NUM_PIXELS; //Number of the highest pixel previously lit

long countDownSeconds = 10; //Arbitrarily set

long countDownMillis = countDownSeconds * 1000; //Number of millis in total countdown time
long previousCountDownMillis = 0;
int stickDifference = 0; //Value at which a pixel should be dropped

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(9600);
  strip.begin();
  strip.clear();
  strip.show();
  startMillis = millis();
  //  int stickCountDown = ((int)map(512, 0, 1023, 0, NUM_PIXELS)); //Pot value mapped to length of stick (NUM_PIXELS)
  //Arbitrary pot value of 50% of range
  stickDifference = countDownMillis / NUM_PIXELS; //How many millis between redisplay of stick
  //  previousCountDownMillis = millis();

}

void loop() {
  //Detect countdown change
  decrementCounter(); //Go see if a stickDifference has occurred

  if (decrementNow == true) { //If we counted down to the point to blank a pixel
    //  if (millis() > (tickMillis + UPDATE_STEP_DELAY)) { //UPDATE_STEP_DELAY to be removed when we trigger update by event
    strip.clear();
    strip.show();
    //    tickMillis = millis(); //Reset the idle count
    isIdle = false;


  }
  //  Serial.println("lastLitPixel = " + (String)lastLitPixel);



  if (!isIdle) {
    updateCountdown(ANIMATION_STEP_DELAY); //Go update the stick

    decrementNow = false; // Change to say we're looking for another pixel to blank

  }


  if (numLitPixels > lastLitPixel) { //Reset the pixels to start again from pixel 0 when refreshing the stick.
    numLitPixels = 0;

    isIdle = true;
  }

}




