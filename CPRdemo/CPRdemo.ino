#ifdef __AVR__
#include <avr/power.h>
#endif

#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include "Button.cpp"
#include "Potentiometer.cpp";
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
bool adultMode = true;

// variables for setup state
int currentBeatsPerMinutePotValue = 0 ;
long totalDistance=0;
int directionChangeCounter = 0;
bool dirPlus = false;
int totalDepth = 0; //in hundredths of an inch
int previousDistanceValue = 0;
int startDistanceValue = 0;

Button adultChildButton = Button(BUTTON_ADULTCHILD, LED_ADULTCHILD, false);
Button startStopButton = Button(BUTTON_STARTSTOP, LED_STARTSTOP);
Potentiometer bpmPot = Potentiometer(POT_PIN_BEATSPERMINUTE, 100);
Potentiometer durationPot = Potentiometer(POT_PIN_TIME, NUM_SAMPLES);

const int MAX_NUM_SECONDS = 90;
const int MIN_NUM_SECONDS = 30;
const int MAX_NUM_HUNDREDTHS = 200;
const int MIN_NUM_HUNDREDTHS = 0;
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
  
//Read the TIME pot 10 times and store the list of values in previousTPVs
  durationPot.init();
  bpmPot.init();
//  for (int i = 0; i < NUM_SAMPLES; i++) {
//   previousTimePotValues[i] =map(analogRead(POT_PIN_TIME), 0, 1023, MIN_NUM_SECONDS, MAX_NUM_SECONDS);
//}


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



//void HandleLitButtonPress(int buttonPin, int ledPin) {
//  digitalWrite(ledPin, !digitalRead(buttonPin));
//}

//**********Add reset for TIME pot value to prevent buffer overflows


void UpdateSetup() {
//Runs continuously when we're in the UpdateSetup state.
// Read and post the time pot value to the display.  
// Change to minutes:seconds format?

  // gets the smoothed value from the pot, then maps it into the 30-90 second range
  redDisplay.print((int)map(bpmPot.getRollingAverage(), 0, 1023, MIN_NUM_SECONDS, MAX_NUM_SECONDS));
  redDisplay.writeDisplay();
  

//The StartStop button moves us to the next state.

  if(startStopButton.wasPressed()) {
    previousDistanceValue = analogRead(POT_PIN_BEATSPERMINUTE);
    startDistanceValue = previousDistanceValue;

    // read the adult/child button at the moment we exit this state and use that value to determine which mode runs in the play state
    adultMode = digitalRead(BUTTON_ADULTCHILD);  // I don't remember whether pressed is adult or pressed is child. Invert if necessary
    GoToNextState();
  }

}


void UpdatePlay() {
  redDisplay.writeDigitNum(0, PLAY);
  redDisplay.writeDisplay();

//Save the time pot value. Already in averageTimePotValue?
//Start countdown to display - Format min:sec with blinking colon.
//Verify whether audio is busy - Save condition
//Read BPM pot. Pass value to each function below
 currentBeatsPerMinutePotValue = bpmPot.getRollingAverage();

//******Need to determine direction and if there's been a change
//******ForBPM, need to count changes and /2 and post to the display every 5 seconds or so.
//******For depth of compressions need to measure distances and /2 amd divide by 100 for inches/compression
//This function to average BPM every 5 seconds


//  
//  greenDisplay.print((int)averagebeatsPerMinuteValue);
//  //Detect change in direction
  //At each 2 changes, add 1 to beats total
  //After 5 seconds, post current 5-second average to display
    //If more than 10 seconds and BPM <100, Check if audio is busy
    //If audio is busy, wait until it's not
      //else play "go faster". Track next 10 seconds.  Need a "busy time" counter for each audio file?
      //Can this be obtained automatically from read of card?
    //else if BPM >100, play "good rate"
    //Add the 5-second beats to total beats
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
//Note: Pot value should increase at beginning

  
  //Serial.println("Current bpm pot val: " + (String)currentBeatsPerMinutePotValue);
  //Serial.println(currentBeatsPerMinutePotValue);
int currentDistanceValue = currentBeatsPerMinutePotValue / 25; // change to variable
if ((currentDistanceValue < previousDistanceValue) && !dirPlus) { //Has direction changed?  If so, going up now.
  Serial.println("Going up");
  startDistanceValue = currentDistanceValue; //Update start distance
  dirPlus = !dirPlus; //Change direction flag
 }

 if ((currentDistanceValue > previousDistanceValue) && dirPlus ) { //Has direction changed?  If so, going down now.
   directionChangeCounter ++; //Add one to count to obtain cycles.
   Serial.println("Going down");
   startDistanceValue = currentDistanceValue; //Update start distance
   dirPlus = !dirPlus; //Change direction
  }

  totalDistance += abs(startDistanceValue - previousDistanceValue);
  previousDistanceValue = currentDistanceValue;
//  Serial.print("Total Distance ");
//  Serial.println(totalDistance);
//  Serial.print("dirPlus is ");
//  Serial.println(dirPlus);
//  Serial.print("directionChangeCounter= ");
//  Serial.println(directionChangeCounter);
  
  



//Look for change in pot value
//If pot value decreases, save last value and add to total depth
  //Measure depth of compression  Need to scale pot value to distance.  Map function
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
 if(startStopButton.wasPressed()) {
  GoToNextState();
 }
  
}

void UpdateFeedback() {
  redDisplay.writeDigitNum(0, FEEDBACK);
  redDisplay.writeDisplay();
//Post BPM to green display
  

  
//Post average depth of compressions to green display.
//  greenDisplay.print((int)averageDistanceValue); 
//  greenDisplay.writeDisplay(); //Jittery?
  
//Play "keep it up until help arrives" 


//The StartStop button moves us to the next state.
  if(startStopButton.wasPressed()) {
  GoToNextState();
 }
}

void UpdateCalibration() {
  redDisplay.writeDigitNum(0, CALIBRATION);
  redDisplay.writeDisplay();

  if(startStopButton.wasPressed()) {
    GoToNextState();
 }
}

// the loop function runs over and over again forever
void loop() {

  // always update inputs, no matter what state we're in
  adultChildButton.updateButton();
  startStopButton.updateButton();
  durationPot.updatePot();
  bpmPot.updatePot();


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




