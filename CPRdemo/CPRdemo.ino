#ifdef __AVR__
#include <avr/power.h>
#endif

#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include "Button.cpp"
#include "Potentiometer.cpp";
#include <RS485_non_blocking.h>
#include <Wire.h>


#define POT_PIN_TIME A0 // Input from TIME pot
#define POT_PIN_BEATSPERMINUTE A1 // Input from BEATSPERMINUTE pot
#define BUTTON_STARTSTOP 2 // Set pin 2 for StartStop button
#define BUTTON_ADULTCHILD 4 // Set pin 4 for AdultChild button
#define NUM_SAMPLES 10
#define LED_STARTSTOP 8 // Set pin 8 for Start/Stop button LED
#define LED_ADULTCHILD 12 //Set pin 12 for Adult/Child button LED
#define LED_AVERAGEBPM 9 // Set pin 8 for Start/Stop button LED
#define LED_OVERALLBPM 10 //Set pin 12 for Adult/Child button LED#define NUM_BPM_SAMPLES 40

Adafruit_7segment redDisplay = Adafruit_7segment();
Adafruit_7segment greenDisplay = Adafruit_7segment();


enum StateID {
  SETUP,
  PLAY,
  FEEDBACK,
  CALIBRATION
};

enum AudioFeedbackMode {
  CHECK_FOR_PACE,
  CHECK_FOR_DEPTH
};

size_t fWrite(const byte what) {
  return Serial.write(what);
}


// variables common to all states
StateID currentState = SETUP;
bool includeCalibration = false;
bool adultMode = true;
int smoothingValue = 15;
Button adultChildButton = Button(BUTTON_ADULTCHILD, LED_ADULTCHILD, false);
Button startStopButton = Button(BUTTON_STARTSTOP, LED_STARTSTOP);
Potentiometer bpmPot = Potentiometer(POT_PIN_BEATSPERMINUTE, 40);
Potentiometer timePot = Potentiometer(POT_PIN_TIME, NUM_SAMPLES);
RS485 commChannel(NULL, NULL, fWrite, 0);

unsigned long previousMillis = 0;        // will store last time LED was updated
const long interval = 1000;           // interval at which to blink (milliseconds) Usually 1000  Why is this long variable?
boolean drawDots = true;  //A variable to hold whether to display dots or not
unsigned long startTime = 0;
unsigned long timeCountDown = startTime;
unsigned long currentMillis = millis();
int seconds; //Actual seconds
int minutes; //Actual minutes
int hours; //Actual hours
const int secsPerHour = 3600;
const int secsPerMinute = 60;
int dotCount = 0; //Counter for doubling dot display speed
const byte chrDot3   = 0b00000100;  // Dot 3 Top, left.  Must be output to character position 2
const byte chrDot4   = 0b00001000;  // Dot 4 Bottom, left.  Must be output to character position 2

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
long overallBpmStartTime = 0;
int overallBpmCounterStart = 0;
int overallSeconds = 0;
int upDistance = 0;
int downDistance = 0;
boolean previousDownWasShort = false;
int shortUpStrokeCounter = 0;
int distanceCounterBeats = 5;
int overallBpmCount = 0;
int audioFeedbackMode = CHECK_FOR_PACE;
bool haveAchievedGoodPace = false; // guilty until proven innocent
bool haveAchievedGoodDepth = false;
bool havePlayedMusic = false;
int numLittleFasterPrompts = 0;

unsigned long previousBlink = millis();

const int BLINK_INTERVAL = 500;
const byte GOOD_COMP [] = "ONE";
const byte RIGHT_SPEED [] = "TWO";
const byte GOT_THIS [] = "THREE";
const byte LITTLE_FASTER [] = "FOUR";
const byte INTRO_AND_MUSIC [] = "FIVE";
const byte PUSH_HARDER [] = "SIX";
const byte MED_HELP [] = "SEVEN";
const byte TIRED [] = "EIGHT";
const byte MUSIC_ONLY [] = "NINE";

const int AVERAGE_BPM_SAMPLE_TIME = 5000;//How long between averaging and postings of averageBpm, in millis().
const int BPM_CONVERT = (60 / (AVERAGE_BPM_SAMPLE_TIME / 1000));
const int MAX_NUM_SECONDS = 182;
const int MIN_NUM_SECONDS = 15;
//const int MAX_NUM_HUNDREDTHS = 200; //Unused
//const int MIN_NUM_HUNDREDTHS = 0; //Unused

//NEW
//Variables for Calibrate state
int maximumDepth = (350 / smoothingValue); //Eventually get this from a read of the bpm pot in the calibrate state.
//new




// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pins.
  pinMode(LED_STARTSTOP, OUTPUT);
  pinMode(LED_ADULTCHILD, OUTPUT);
  pinMode(LED_AVERAGEBPM, OUTPUT);
  pinMode(LED_OVERALLBPM, OUTPUT);
  pinMode(BUTTON_STARTSTOP, INPUT_PULLUP);
  pinMode(BUTTON_ADULTCHILD, INPUT_PULLUP);

  greenDisplay.begin(0x70);
  redDisplay.begin(0x71);


  //Clear the displays and set brightness
  greenDisplay.clear();
  greenDisplay.writeDisplay();
  redDisplay.clear();
  redDisplay.writeDisplay();
  redDisplay.drawColon(true);  //Why is this blnking like mad?
  redDisplay.writeDisplay();

  redDisplay.setBrightness (15);  //Values 0-15
  greenDisplay.setBrightness (15);  //Values 0-15

  Serial.begin(9600);

  commChannel.begin();

  timePot.init();
  bpmPot.init();

} //End setup


//This is the GoToNextState function.
void GoToNextState()
{
  Serial.println("Old state id: " + (String)currentState);
  int newStateId = (currentState + 1) % (includeCalibration ? 4 : 3);

  Serial.println("New state id: " + (String)newStateId);
  Serial.println();
  currentState = newStateId;
}
//End of GoToNextState function


void UpdateSetup() {
  // gets the smoothed value from the time pot, then maps it into the min-max second range

  timeCountDown = ((int)map(timePot.getRollingAverage(), 0, 1023, MIN_NUM_SECONDS, MAX_NUM_SECONDS));
  handleStartTimeConvert();

  if (startStopButton.wasPressed()) {

    drawDots = false;
    greenDisplay.clear();
    greenDisplay.writeDisplay();

    previousDistanceValue = bpmPot.getRollingAverage() / smoothingValue;

    startDistanceValue = previousDistanceValue;
    averageBpmStartTime = millis();
    overallBpmStartTime = millis();
    averageBpmCounterStart = 0;
    overallBpmCounterStart = beatCounter;
    audioFeedbackMode = CHECK_FOR_PACE;
    haveAchievedGoodPace = false; // guilty until proven innocent
    haveAchievedGoodDepth = false;
    havePlayedMusic = false;
    numLittleFasterPrompts = 0;

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

  handleTimeUpdate(currentMillis);
  handleColonBlink(currentMillis);



  //What to do if person simply stops? Operator presses Stop?
  //Or autodetect something like 2 compressions missed and advance to the next state?

  int currentDistanceValue = bpmPot.getRollingAverage() / smoothingValue; // change to variable  What if pot is zero
  checkForDirectionChange(currentDistanceValue);
  calculateAverageBPM();

  previousDistanceValue = currentDistanceValue;
  //    if (averageBpm < 95) {
  //      commChannel.sendMsg(THREE, sizeof(THREE));
  //    }
  //    if (averageBpm > 100) {
  //      commChannel.sendMsg(NINE, sizeof(NINE));
  //    }

  if (startStopButton.wasPressed() || (timeCountDown + 1 == 0)) {
    overallBpmCount = beatCounter - overallBpmCounterStart;
    overallSeconds = (millis() - overallBpmStartTime) / 1000;
    digitalWrite(LED_AVERAGEBPM, LOW);

    digitalWrite(LED_OVERALLBPM, HIGH);

    greenDisplay.print((overallBpmCount * secsPerMinute) / overallSeconds);
    greenDisplay.writeDigitRaw (2, chrDot4); //Bottom left dot
    greenDisplay.writeDisplay();
    GoToNextState();

  }
}


void UpdateFeedback() {
  //  redDisplay.writeDigitNum(0, FEEDBACK);
  //  redDisplay.writeDisplay();

  beatCounter = 0;


  if (startStopButton.wasPressed()) {

    greenDisplay.clear();
    greenDisplay.writeDisplay();
    digitalWrite(LED_OVERALLBPM, LOW);
    GoToNextState();
  }
}

void UpdateCalibration() {
  redDisplay.writeDigitNum(0, CALIBRATION);
  redDisplay.writeDigitRaw(2, chrDot3);
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

  if (startStopButton.wasPressed()) {

    GoToNextState();
  }

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

/* Notes from 8/4
  Moved includeCalibration into global variables area so we can do some sort of button sequence to read to
  change the variable from false to true so we can get into calibration state without a computer connected.
  Added average BPM for the entire test time
*/

/* Notes from 8/12
    Fixed overall BPM calcs
    Removed overall BPM function
    Added code for adding average and overall BPM LEDs.  Ready for hardware.
*/
