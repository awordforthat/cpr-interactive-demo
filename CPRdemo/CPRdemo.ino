#ifdef __AVR__
#include <avr/power.h>
#endif

#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <Wire.h>
//#include <WaveHC.h>
//#include <WaveUtil.h>

#define POT_PIN_TIME A0 // Input from TIME pot
#define POT_PIN_BEATSPERMINUTE A1 // Input from BEATSPERMINUTE pot
#define LED_PIN 11 // Unused right now
#define BUTTON_STARTSTOP 2 // Set pin 2 for StartStop button
#define BUTTON_ADULTCHILD 4 // Set pin 4 for AdultChild button
#define BUTTON_STEPSTATE 5 // Set pin 5 for StepState button
#define NUM_SAMPLES 10
#define LED_STARTSTOP 8 // Set pin 8 for Start/Stop button LED
#define LED_ADULTCHILD 12 //Set pin 12 for Adult/Child button LED

Adafruit_7segment redDisplay = Adafruit_7segment();
Adafruit_7segment greenDisplay = Adafruit_7segment();


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
  
  // initialize digital pins.
	pinMode(LED_PIN, OUTPUT); //Unused now
	pinMode(LED_BUILTIN, OUTPUT); //Unused now
	pinMode(LED_STARTSTOP, OUTPUT);
	pinMode(LED_ADULTCHILD, OUTPUT);
	pinMode(BUTTON_STARTSTOP, INPUT_PULLUP);
  pinMode(BUTTON_STEPSTATE, INPUT_PULLUP); //Unused now
	pinMode(BUTTON_ADULTCHILD, INPUT_PULLUP);
  
  greenDisplay.begin(0x70);
  redDisplay.begin(0x71);

	//Clear the displays
  greenDisplay.clear();
  greenDisplay.writeDisplay();
  redDisplay.clear();
  redDisplay.writeDisplay();
  
  Serial.begin(9600);
  
//Read the TIME pot and post it to the display once.
  for (int i = 0; i < NUM_SAMPLES; i++) {
   previousTimePotValues[i] =map(analogRead(POT_PIN_TIME), 0, 1023, MIN_NUM_SECONDS, MAX_NUM_SECONDS);
}
} //End setup 


//This is the debounce function.  Runs when called.
boolean CheckDebounce(int instReading, int triggerVal = LOW)
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
//End debounce function

//This is the GoToNextState function.
void GoToNextState(bool includeCalibration = false)
{
  Serial.println("Old state id: " + (String)currentState);
  int newStateId = (currentState + 1) % (includeCalibration ? 4 : 3);
 
  Serial.println("New state id: " + (String)newStateId);
  Serial.println();
  currentState = newStateId;
}
//End of GoToNextState function


// Light GreenLed function
void GreenLed()
{
  //Light the StartStop LED when STARTSTOP button is pressed
  int buttonStartStopVal = 0; 
  int instReading = digitalRead(BUTTON_STARTSTOP);  
  buttonStartStopVal = instReading;
  buttonStartStopVal = !buttonStartStopVal; //Invert button state to make turn LED on when button is depressed
  digitalWrite(LED_STARTSTOP, buttonStartStopVal); //Light the green LED when STARTSTOP is pressed.
}
//End Light GreenLed function


// Light YellowLed function
void YellowLed()
{
  //Light the Adult/Child LED when ADULTCHILD button is pressed
  int buttonAdultChildVal = 0; 
  int instReading = digitalRead(BUTTON_ADULTCHILD);  
  buttonAdultChildVal = instReading;
  buttonAdultChildVal = !buttonAdultChildVal; //Invert button state to make turn LED on when button is depressed
  digitalWrite(LED_ADULTCHILD, buttonAdultChildVal); //Light the yellow LED when ADULTCHILD is pressed.
}
//End Light YellowLed function

//**********Add reset for TIME pot value to prevent buffer overflows


void UpdateSetup() {
//Runs continuously when we're in the UpdateSetup state.
// Read and post the time pot value to the display.  Change to minutes:seconds format?
int averageTimePotValue = 0;
  for(int i = 0; i < NUM_SAMPLES; i++) {
    averageTimePotValue += previousTimePotValues[i];
  }
  averageTimePotValue = (int)averageTimePotValue/NUM_SAMPLES;
  previousTimePotValues[currentTimePotIndex % NUM_SAMPLES] = map(analogRead(POT_PIN_TIME), 0, 1023, MIN_NUM_SECONDS, MAX_NUM_SECONDS);
  currentTimePotIndex = currentTimePotIndex + 1;
  
  redDisplay.print((int)averageTimePotValue);
  redDisplay.writeDisplay();
  

//The StartStop button moves us to the next state.

int instReading = digitalRead(BUTTON_STARTSTOP);
 if(CheckDebounce(instReading)) //if switch is stable (debounced)
  
  {
    GoToNextState(true); //go to PLAY state
  }

}


void UpdatePlay() {
  redDisplay.writeDigitNum(0, PLAY);
  redDisplay.writeDisplay();
  
//The StartStop button moves us to the next state.
  int instReading = digitalRead(BUTTON_STARTSTOP);
  if(CheckDebounce(instReading)) //if switch is stable (debounced)
  {
    GoToNextState(true); //go to FEEDBACK state
  }
  
}

void UpdateFeedback() {
  redDisplay.writeDigitNum(0, FEEDBACK);
  redDisplay.writeDisplay();

//The StartStop button moves us to the next state.
  int instReading = digitalRead(BUTTON_STARTSTOP);
  if(CheckDebounce(instReading)) //if switch is stable (debounced)
  {
    GoToNextState(true); //go to CALIBRATION state
  }
}

void UpdateCalibration() {
  redDisplay.writeDigitNum(0, CALIBRATION);
  redDisplay.writeDisplay();

    int instReading = digitalRead(BUTTON_STARTSTOP);
  if(CheckDebounce(instReading)) //if switch is stable (debounced)
  {
    GoToNextState(true); //go to SETUP state
  }
}

// the loop function runs over and over again forever
void loop() {
  GreenLed();
  YellowLed();


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
      break;
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




