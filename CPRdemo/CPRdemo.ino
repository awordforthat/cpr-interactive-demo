#ifdef __AVR__
#include <avr/power.h>
#endif

#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include "Button.cpp"
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
StateID currentState = SETUP;

// variables for setup state
int previousTimePotValues[NUM_SAMPLES];
int currentTimePotIndex = 0;
Button adultChildButton = Button(BUTTON_ADULTCHILD, LED_ADULTCHILD);
Button startButton = Button(BUTTON_STARTSTOP, LED_STARTSTOP);
const int MAX_NUM_SECONDS = 90;
const int MIN_NUM_SECONDS = 30;
 
// the setup function runs once when you press reset or power the board
void setup() {
  
  // initialize digital pins.
	pinMode(LED_STARTSTOP, OUTPUT);
	pinMode(LED_ADULTCHILD, OUTPUT);
	pinMode(BUTTON_STARTSTOP, INPUT_PULLUP);
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
//void GreenLed()
//{
//  //Light the StartStop LED when STARTSTOP button is pressed
//  int buttonStartStopVal = 0; 
//  int instReading = digitalRead(BUTTON_STARTSTOP);  
//  buttonStartStopVal = instReading;
//  buttonStartStopVal = !buttonStartStopVal; //Invert button state to make turn LED on when button is depressed
//  digitalWrite(LED_STARTSTOP, buttonStartStopVal); //Light the green LED when STARTSTOP is pressed.
//}
////End Light GreenLed function
//
//
//// Light YellowLed function
//void YellowLed()
//{
//  //Light the Adult/Child LED when ADULTCHILD button is pressed
//  int buttonAdultChildVal = 0; 
//  int instReading = digitalRead(BUTTON_ADULTCHILD);  
//  buttonAdultChildVal = instReading;
//  buttonAdultChildVal = !buttonAdultChildVal; //Invert button state to make turn LED on when button is depressed
//  digitalWrite(LED_ADULTCHILD, buttonAdultChildVal); //Light the yellow LED when ADULTCHILD is pressed.
//}
////End Light YellowLed function
//
//void HandleLitButtonPress(int buttonPin, int ledPin) {
//  digitalWrite(ledPin, !digitalRead(buttonPin));
//}

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

  if(startButton.wasPressed()) {
    GoToNextState();
  }

}


void UpdatePlay() {
  redDisplay.writeDigitNum(0, PLAY);
  redDisplay.writeDisplay();
//Save the time pot value
//Start countdown to display
//Verify whether audio is busy - Save condition
//Read BPM pot. Pass value to each function below

//One function to average BPM every 5 seconds
  //Detect change in direction
  //At each 2 changes, add 1 to beats total
  //After 5 seconds, post current 5-second average to display
    //If more than 10 seconds and BPM <100, Check if audio is busy
    //If audio is busy, wait until it's not
      //else play "go faster". Track next 10 seconds.
    //else if BPM >100, play "good rate"
    //Add 5-second beats to total beats
    //If more than 20 seconds and BPM <100, play "Stayin' Alive intro, then Stayin' Alive
    //If 10 seconds at right rate, lower volume of music or fade it out and set audio to not busy.
    
  //Save number of 5 second periods for final BPM calculation
  //Add each beat to totalBpm
  //Calculate beats/minute
  //Post to display
//What to do if person simply stops? Operator presses Stop?  
//Or autodetect something like 2 compressions missed and advance to the next state?
  
//One function to check depth of compressions
// Save current pot value to measure depth
//Pot value should increase at beginning
//Look for change in pot value
//If pot value decreases, save last value and add to total depth
  //Measure depth of compression
//Look for change in pot value
//If pot value increases, save last value and add to total depth
  //Measure depth of compression
//If pot value returned to starting value within a tolerance range - OK
  //Else note shortcoming and save to trigger audio
//If more than 10 seconds at too shallow, play "Too Shallow" audio
//  else, play "good depth" audio.

//


//If STARTSTOP is pressed wrap it up, then advance to Feedback

  
//The StartStop button moves us to the next state.
 if(startButton.wasPressed()) {
  GoToNextState();
 }
  
}

void UpdateFeedback() {
  redDisplay.writeDigitNum(0, FEEDBACK);
  redDisplay.writeDisplay();
//Post BPM to green display
//Post average depth of compressions to red display.
//Play "keep it up until help arrives" 


//The StartStop button moves us to the next state.
  if(startButton.wasPressed()) {
  GoToNextState();
 }
}

void UpdateCalibration() {
  redDisplay.writeDigitNum(0, CALIBRATION);
  redDisplay.writeDisplay();

  if(startButton.wasPressed()) {
    GoToNextState();
 }
}

// the loop function runs over and over again forever
void loop() {

  adultChildButton.updateButton();
  startButton.updateButton();

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




