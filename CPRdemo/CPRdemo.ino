#ifdef __AVR__
#include <avr/power.h>
#endif

#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <Wire.h>

#define POT_PIN A0 // I'm pretty sure this is an analog IO pin - if you're having trouble reading from it, maybe change this
#define LED_PIN 11 // this is definitely digital IO - this is fine
#define BUTTON_PIN 7 // temporary to test state changes
#define NUM_SAMPLES 10

Adafruit_7segment redDisplay = Adafruit_7segment();
Adafruit_7segment greenDisplay = Adafruit_7segment();

// temporary change


enum StateID {
  SETUP,
  PLAY,
  FEEDBACK,
  CALIBRATION
};

// variables common to all states
int currentInstantaneousButtonState = 0;
int prevInstantaneousButtonState = 0;
int prevRealButtonState = 0;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
StateID currentState = SETUP;

// variables for setup state
int previousTimePotValues[NUM_SAMPLES];
int currentTimePotIndex = 0;
const int MAX_NUM_SECONDS = 90;
const int MIN_NUM_SECONDS = 30;
 
// the setup function runs once when you press reset or power the board
void setup() {
  
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(7, INPUT);

  redDisplay.begin(0x70);
  greenDisplay.begin(0x71);
  
  Serial.begin(9600);

  for (int i = 0; i < NUM_SAMPLES; i++) {
   previousTimePotValues[i] =map(analogRead(POT_PIN), 0, 1023, MIN_NUM_SECONDS, MAX_NUM_SECONDS);
}
}

boolean CheckDebounce(int instReading, int triggerVal = HIGH)
{
  boolean returnable = false; // whether or not the input shows a real HIGH value
  
  if(instReading != prevInstantaneousButtonState)
  {
    lastDebounceTime = millis();
  } 

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // in here should be the real reading
     if(instReading == triggerVal && prevRealButtonState == !triggerVal)
     {
       // inside this loop, the trigger value has been solid for 50ms, && the previous real value was the opposite of the trigger
       returnable = true;
     }
     // record the real value for comparing to later
      prevRealButtonState = instReading;
  }

  // record the instantaneous reading for the next debounce comparison
  prevInstantaneousButtonState = instReading;

  return returnable;
}

void GoToNextState(bool includeCalibration = false)
{
  Serial.println("Old state id: " + currentState);
  int newStateId = (currentState + 1) % (includeCalibration ? 4 : 3);
  Serial.println("New state id: " + (String)newStateId);
  currentState = newStateId;
}

void UpdateSetup() {
  int averageTimePotValue = 0;
  for(int i = 0; i < NUM_SAMPLES; i++) {
    averageTimePotValue += previousTimePotValues[i];
  }
  averageTimePotValue = (int)averageTimePotValue/NUM_SAMPLES;
  previousTimePotValues[currentTimePotIndex % NUM_SAMPLES] = map(analogRead(POT_PIN), 0, 1023, MIN_NUM_SECONDS, MAX_NUM_SECONDS);
  currentTimePotIndex = currentTimePotIndex + 1;
  
  redDisplay.print((int)averageTimePotValue);
  redDisplay.writeDisplay();
  
}

void UpdatePlay() {
  redDisplay.writeDigitNum(0, PLAY);
  redDisplay.writeDisplay();
}

void UpdateFeedback() {
  redDisplay.writeDigitNum(0, FEEDBACK);
  redDisplay.writeDisplay();
}

void UpdateCalibration() {
  redDisplay.writeDigitNum(0, CALIBRATION);
  redDisplay.writeDisplay();
}

// the loop function runs over and over again forever
void loop() {
  
  int instReading = digitalRead(BUTTON_PIN);

  if(CheckDebounce(instReading))
  {
    GoToNextState(true);
  }

  bool error = false;
  switch(currentState) {
    case SETUP:
      UpdateSetup();
      break;
    case PLAY:
      UpdatePlay();
      break;
    case FEEDBACK:
      UpdateFeedback();
    case CALIBRATION:
      UpdateCalibration();
      break;
    default:
      error = true;

    if(error) {
      Serial.println("State " + (String)currentState + " was unrecognized");
    }
  }

 

}




