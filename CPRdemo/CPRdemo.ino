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
#define BUTTON_STARTSTOP 2 // Set pin 2 for StartStop button
#define BUTTON_ADULTCHILD 4 // Set pin 4 for AdultChild button
#define NUM_SAMPLES 10
#define LED_STARTSTOP 8 // Set pin 8 for Start/Stop button LED
#define LED_ADULTCHILD 12 //Set pin 12 for Adult/Child button LED
#define NUM_BPM_SAMPLES 40

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
int smoothingValue = 15;
Button adultChildButton = Button(BUTTON_ADULTCHILD, LED_ADULTCHILD, false);
Button startStopButton = Button(BUTTON_STARTSTOP, LED_STARTSTOP);
Potentiometer bpmPot = Potentiometer(POT_PIN_BEATSPERMINUTE, 40);
Potentiometer timePot = Potentiometer(POT_PIN_TIME, NUM_SAMPLES);

//***********
unsigned long previousMillis = 0;        // will store last time LED was updated
const long interval = 1000;           // interval at which to blink (milliseconds) Usually 1000  Why is this long variable?
boolean drawDots = false;  //A variable to hold whether to display dots or not
unsigned long startTime = 0;
unsigned long timeCountDown = startTime;
unsigned long currentMillis = millis();
int seconds; //Actual seconds
int minutes; //Actual minutes
int hours; //Actual hours
const int secsPerHour = 3600;
const int secsPerMinute = 60;
int dotCount = 0; //Counter for doubling dot display speed
//************

// variables for setup state


// variables for play state
long totalDistance = 0; //Try to divide the source so an int can be used.
int directionChangeCounter = 0;
bool dirPlus = false;
int totalDepth = 0; //in hundredths of an inch?
int previousDistanceValue = 0;
int startDistanceValue = 0;
int averageBpm = 0;
int beatCounter = 0;
long averageBpmStartTime = 0;
int averageBpmCounterStart = 0;
int upDistance = 0;
int downDistance = 0;
boolean previousDownWasShort = false;
int shortUpStrokeCounter = 0;
int distanceCounterBeats = 5;
//**********
unsigned long previousBlink = millis();

const int BLINK_INTERVAL = 500;
//**********

const int AVERAGE_BPM_SAMPLE_TIME = 5000;//How long between averaging and postings of averageBpm, in millis().
const int BPM_CONVERT = (60 / (AVERAGE_BPM_SAMPLE_TIME / 1000));
const int MAX_NUM_SECONDS = 90;
const int MIN_NUM_SECONDS = 10;
const int MAX_NUM_HUNDREDTHS = 200;
const int MIN_NUM_HUNDREDTHS = 0;

//NEW
//Variables for Calibrate state
int maximumDepth = (1023 / smoothingValue); //Eventually get this from a read of the bpm pot.
//new

// TODO: optimize memory by changing variable types to the smallest unit that will accommodate their range of values
// Variable size reference: https://learn.sparkfun.com/tutorials/data-types-in-arduino


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pins.
  pinMode(LED_STARTSTOP, OUTPUT);
  pinMode(LED_ADULTCHILD, OUTPUT);
  pinMode(BUTTON_STARTSTOP, INPUT_PULLUP);
  pinMode(BUTTON_ADULTCHILD, INPUT_PULLUP);

  greenDisplay.begin(0x70);
  redDisplay.begin(0x71);


  //Clear the displays and set brightness

  greenDisplay.clear();
  greenDisplay.writeDisplay();
  redDisplay.clear();
  redDisplay.writeDisplay();

  //**********
  redDisplay.setBrightness (8);  //Values 0-15
  greenDisplay.setBrightness (15);  //Values 0-15
  //**********

  Serial.begin(9600);

  timePot.init();
  bpmPot.init();
  int bpmValue = analogRead(POT_PIN_BEATSPERMINUTE);//Unused
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


void UpdateSetup() {
  // TODO: Change to minutes:seconds format?

  // gets the smoothed value from the time pot, then maps it into the min-max second range
  redDisplay.print((int)map(timePot.getRollingAverage(), 0, 1023, MIN_NUM_SECONDS, MAX_NUM_SECONDS));
  redDisplay.writeDisplay();
  greenDisplay.clear();
  greenDisplay.writeDisplay();

  if (startStopButton.wasPressed()) {

    timeCountDown = map(timePot.getRollingAverage(), 0, 1023, MIN_NUM_SECONDS, MAX_NUM_SECONDS);
    Serial.println("timeCountDown= " + (String)timeCountDown);

    previousDistanceValue = bpmPot.getRollingAverage() / smoothingValue;
    Serial.println("Prev dist val: " + (String)previousDistanceValue);
    startDistanceValue = previousDistanceValue;
    averageBpmStartTime = millis();

    // read the adult/child button at the moment we exit this state and use that value to determine which mode runs in the play state
    adultMode = digitalRead(BUTTON_ADULTCHILD);  // 1= Adult, 0= Child
    if (adultMode == 1)
    {
      Serial.println("adultMode= ADULT");
    }
    else
    {
      Serial.println("adultMode= CHILD");
    }

    GoToNextState();
  }
}

void UpdatePlay() {
  if (seconds < 10) {
    redDisplay.writeDigitNum(3, 0);
    redDisplay.writeDisplay();
  }

  currentMillis = millis(); //Record current time (used in calculating what to display on each of the 7-segs)

  handleColonBlink(currentMillis);

  handleTimeUpdate(currentMillis);

  //What to do if person simply stops? Operator presses Stop?
  //Or autodetect something like 2 compressions missed and advance to the next state?

  int currentDistanceValue = bpmPot.getRollingAverage() / smoothingValue; // change to variable  What if pot is zero
  checkForDirectionChange(currentDistanceValue);
  calculateAverageBPM();

  previousDistanceValue = currentDistanceValue;

  if (startStopButton.wasPressed() || (timeCountDown + 1 == 0)) {
    GoToNextState();
  }
}


void UpdateFeedback() {
  redDisplay.writeDigitNum(0, FEEDBACK);
  redDisplay.writeDisplay();
  //Post BPM to green display
  greenDisplay.print(averageBpm);
  greenDisplay.writeDisplay();
  beatCounter = 0;

  //Play "keep it up until help arrives"


  if (startStopButton.wasPressed()) {
    GoToNextState();
  }
}

void UpdateCalibration() {
  redDisplay.writeDigitNum(0, CALIBRATION);
  redDisplay.writeDisplay();


  //NEW
  //Send some message to redDisplay? "Prss Dn"?
  //    maximumDepth = (bpmPot.getRollingAverage() + smoothingValue); //Create a value for maximumDepth a little larger than the actual pot value.
  // Used to scale pot distance for inches with map function
  //    Serial.println("new maximumDepth= " + (String)maximumDepth);
  //new
  //Need a way to signal the operator to press, etc.  For now, just hard code it. (273)
  //    if (startStopButton.wasPressed()) {
  //      greenDisplay.clear();
  //      greenDisplay.writeDisplay();  //NEW

  GoToNextState();

  //    }
}

// the loop function runs over and over again forever
void loop() {

  // always update inputs, no matter what state we're in
  adultChildButton.updateButton();
  startStopButton.updateButton();
  timePot.updatePot();
  bpmPot.updatePot();

  bool error = false;
  switch (currentState) {
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

      if (error) {
        Serial.println("State " + (String)currentState + " was unrecognized");
      }
  }
}





/* Notes from 8/1
  Blank green display after feedback
  Change smoothing to variable set at 15 from 25
  Test for adequate depth
  Reduced pot stroke mechanically and changed max depth to match
  Moving pot slowly causes lots of false reads
  Removed a ton of commented-out old stuff
  Em added distance counter code
*/

/* Notes from 8/2
  Added halfStrokeSamples variable
  Reset beatCounter to 0 at end of feedback state
*/
