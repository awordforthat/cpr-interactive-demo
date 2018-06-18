#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define POT_PIN 1 // I'm pretty sure this is an analog IO pin - if you're having trouble reading from it, maybe change this
#define LED_PIN 11 // this is definitely digital IO - this is fine
#define NUM_LEDS 10;  // change this to however many LEDs are in that strip you have

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  strip.begin(); 
  strip.setBrightness(255);
  strip.show(); // initialize all pixels to "off"

}

// the loop function runs over and over again forever
void loop() {
  // read the value of the potentiometer
  potVal = analogRead(potPin);
  Serial.println("The potentiometer reading was: " + potVal);

  //strip.setBrightness(???);  // brightness ranges from 0 to 255. replace ??? with math to scale pot value into this range

  // note: setBrightness is not really meant to be used like this, but it'll do for a starter example
  
}

void setStripColor(r, g, b)
{
  for(int i = 0; i < NUM_LEDS; i++)
  {
    strip.setPixelColor(i, r, g, b);
  }
  strip.show();
}

