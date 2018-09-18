/* Written by Emily Charles and Jeff Wachtel. But mostly Emily.
    This code runs a system which is used by the
    Agoura Hills Community Emergency Response Team (AHCERT)
    to train people to perform Hands-Only CPR (Cardio Pulmonary Resuscitation).
    The user selects the time they want to try it out and once they start,
    the system takes input from a pot mounted in a CPR dummy and
    uses its output to determine number of beats per minute
    and depth of compressions. It responds with audio prompts to
    guide the user to do CPR correctly.
*/

#ifdef __AVR__5000

#include <avr/power.h>
#endif

#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include "Button.cpp"
#include "Potentiometer.cpp";
#include <Adafruit_NeoPixel.h>
#include <RS485_non_blocking.h>
#include <Wire.h>


#define POT_PIN_TIME A0 // Input from TIME pot
#define POT_PIN_BEATSPERMINUTE A3 // Input from BEATSPERMINUTE pot
#define BUTTON_STARTSTOP 2 // Set pin 2 for StartStop button
#define BUTTON_ADULTCHILD 4 // Set pin 4 for AdultChild button
#define NUM_SAMPLES 10
#define LED_STARTSTOP 8 // Set pin 8 for Start/Stop button LED
#define LED_ADULTCHILD 12 //Set pin 12 for Adult/Child button LED
#define LED_AVERAGEBPM 9 // Set pin 8 for Start/Stop button LED
#define LED_OVERALLBPM 10 //Set pin 12 for Adult/Child button LED#define NUM_BPM_SAMPLES 40
#define STRIP_PIN 6 //Set pin 6 for Neopixel strip

Adafruit_7segment redDisplay = Adafruit_7segment();
//Adafruit_7segment greenDisplay = Adafruit_7segment();


enum StateID {
  SETUP,
  WAITING,
  PLAY,
  FEEDBACK,
  CALIBRATION
};



enum AudioFeedbackMode {
  CHECK_FOR_PACE_SLOW, // user is going at too slow a pace. Coach them through it until mistake is resolved.
  CHECK_FOR_PACE_FAST, // user is going at too fast a pace. Coach them through it until mistake is resolved.
  CHECK_FOR_DEPTH, // user has made a depth mistake. Coach them through it until mistake is resolved.
  LISTENING // waiting for something to go wrong to kick us into pace or depth mode
};

size_t fWrite(const byte what) {
  return Serial.write(what);
}


// variables common to all states
StateID currentState = SETUP;
bool includeCalibration = false;
bool adultMode = true;
int smoothingValue = 15; //With pot change should we increase this value?
Button adultChildButton = Button(BUTTON_ADULTCHILD, LED_ADULTCHILD, false);
Button startStopButton = Button(BUTTON_STARTSTOP, LED_STARTSTOP);
Potentiometer bpmPot = Potentiometer(POT_PIN_BEATSPERMINUTE, 40);
Potentiometer timePot = Potentiometer(POT_PIN_TIME, NUM_SAMPLES);
RS485 commChannel(NULL, NULL, fWrite, 0);

unsigned long previousMillis = 0;        // will store last time LED was updated
const long interval = 1000;           // interval at which to blink (milliseconds) Usually 1000  Why is this a long variable? Because it calcs with other longs?
//boolean drawDots = true;  //A variable to hold whether to display dots or not
unsigned long startTime = 0;
unsigned long timeCountDown = startTime;
int seconds; //Actual seconds
int minutes; //Actual minutes
int hours; //Actual hours
const int SECS_PER_HOUR = 3600;
const int SECS_PER_MINUTE = 60;
//const byte CHR_DOT_3   = 0b00000100;  // Dot 3 Top, left.  Must be output to character position 2
//const byte CHR_DOT_4   = 0b00001000;  // Dot 4 Bottom, left.  Must be output to character position 2

// variables for setup state


// variables for play state
long totalDistance = 0; //Try to divide the source so an int can be used.
int directionChangeCounter = 0;
bool dirPlus = false;
int previousDistanceValue = 0;
int startDistanceValue = 0;
int averageBpm = 0;
int beatCounter = 0;
long averageIntervalStartTime = 0;
int playDuration;
int averageBpmCounterStart = 0;
long overallBpmStartTime = 0;
int overallBpmCounterStart = 0;
int overallSeconds = 0;
int upDistance = 0;
int downDistance = 0;
bool previousDownWasShort = false;
bool previousUpWasShort = false;
int shortUpStrokeCounter = 0;
int overallBpmCount = 0;
int feedbackMode = -1;
int numCorrections = 0;
int numBadDowns = 0;
int numIntervalBeats = 0;
const int NUM_PIXELS = 24;
int numLitPixels = 0; //Number of pixels to be lit upon updating the stick
int lastLitPixel = NUM_PIXELS; //Number of the highest pixel previously lit
int stickDifference = 0; //Value at which a pixel should be dropped
bool sentFeedbackLastTime = true;
bool sent75pctInfo = false;
bool isIdle = true;
bool decrementNow = false;
//long countDownSeconds = 10; //Arbitrarily set.  To be deleted.
//long countDownMillis = countDownSeconds * 1000; //Number of millis in total countdown time
long previousCountDownMillis = 0;

unsigned long previousBlink = millis();
unsigned long startMillis = 0; //For updateCountdown
int ANIMATION_STEP_DELAY = 15; //Time between stick pixels when refreshing stick
//const int BLINK_INTERVAL = 500;
const byte GOOD_COMP [] = "1";
const byte RIGHT_SPEED [] = "2";
const byte GOT_THIS [] = "3";
const byte LITTLE_FASTER [] = "4";
const byte LITTLE_FASTER_AND_MUSIC [] = "5";
const byte PUSH_HARDER [] = "6";
const byte MED_HELP [] = "7";
const byte TIRED [] = "8";
const byte LITTLE_SLOWER [] = "9";
const byte LITTLE_SLOWER_AND_MUSIC [] = "10";

const int AVERAGE_INTERVAL_SAMPLE_TIME = 5000;//How long between averaging and postings of averageBpm, in millis().
const int BPM_CONVERT = (60 / (AVERAGE_INTERVAL_SAMPLE_TIME / 1000));
const int MAX_NUM_SECONDS = 182;
const int MIN_NUM_SECONDS = 60;
const int MAX_NUM_CORRECTIONS = 2;
const int MIN_ACCEPTABLE_BPM = 100;
const int MAX_ACCEPTABLE_BPM = 121;
const int MIN_STROKE_DISTANCE = 5;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, STRIP_PIN, NEO_GRB + NEO_KHZ800);

//Variables for Calibrate state

int maximumDepth = (750 / smoothingValue); //Also get this from a read of the bpm pot in the calibrate state.
//int minimumDepth = 0; Will we use this?
int calibrateMaximumDepth = 0;
int calibrateMinimumDepth = 1023;



void handleTimeUpdate(long currentMillis) {
  if (currentMillis - previousMillis >= interval) {
    // save the last time you updated the display
    previousMillis = currentMillis;

    //If one second has elapsed, do all these things
    //    hours = (timeCountDown - (timeCountDown % SECS_PER_HOUR)) / SECS_PER_HOUR;
    minutes = ((timeCountDown - (timeCountDown % SECS_PER_MINUTE) - (hours * SECS_PER_HOUR))) / SECS_PER_MINUTE; // SECS_PER_MINUTE;
    seconds = ((timeCountDown % SECS_PER_HOUR) % SECS_PER_MINUTE);
    if (minutes == 0) {
      if (seconds < 10) {
        //        greenDisplay.writeDigitNum(3, 0);
      }
    }

    int displayValue = (minutes * 100) + seconds; //To be pushed to the display.
    //    greenDisplay.print(displayValue);
    //    greenDisplay.writeDisplay();

    timeCountDown--; //Decrement countdown counter
  }
}

//void handleColonBlink(long currentMillis) {
//  if (currentMillis - previousBlink >= BLINK_INTERVAL) {
//    // If a half second has elapsed, do all these things
//    previousBlink = currentMillis;
//    //    greenDisplay.drawColon(drawDots);
//    //    greenDisplay.writeDisplay();
//    drawDots = !drawDots; //invert drawDots state
//  }
//}


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pins.
  pinMode(LED_STARTSTOP, OUTPUT);
  pinMode(LED_ADULTCHILD, OUTPUT);
  pinMode(LED_AVERAGEBPM, OUTPUT);
  pinMode(LED_OVERALLBPM, OUTPUT);
  pinMode(BUTTON_STARTSTOP, INPUT_PULLUP);
  pinMode(BUTTON_ADULTCHILD, INPUT_PULLUP);

  //  greenDisplay.begin(0x70);
  redDisplay.begin(0x71);


  //Clear the displays and set brightness
  //  greenDisplay.clear();
  //  greenDisplay.writeDisplay();
  redDisplay.clear();
  redDisplay.writeDisplay();
  redDisplay.drawColon(true);
  redDisplay.writeDisplay();

  redDisplay.setBrightness (15);  //Values 0-15
  //  greenDisplay.setBrightness (15);  //Values 0-15

  Serial.begin(9600);

  commChannel.begin();
  strip.begin();
  strip.clear();
  strip.show();
  startMillis = millis();
  timePot.init();
  bpmPot.init();

  if (digitalRead(BUTTON_ADULTCHILD) == 0) { //If we power up or reset with the Adult/Child button pressed,
    //we immediately enter the calibrate state.  This may be temporary.
    Serial.println("Calibrate!");
    currentState = 4;
  }

} //End setup


void GoToNextState()
{
  Serial.println("Old state id: " + (String)currentState);
  int newStateId = (currentState + 1) % (includeCalibration ? 5 : 4);
  Serial.println("New state id: " + (String)newStateId);
  Serial.println();
  currentState = newStateId;
}


void UpdateSetup() {
  // gets the smoothed value from the time pot, then maps it into the min-max second range
  // tiny change
  timeCountDown = ((int)map(timePot.getRollingAverage(), 0, 1023, MIN_NUM_SECONDS, MAX_NUM_SECONDS));
  long countDownMillis = timeCountDown * 1000;

  //  int stickCountDown = ((int)map(512, 0, 1023, 0, NUM_PIXELS)); //Pot value mapped to length of stick (NUM_PIXELS)
  //Arbitrary pot value of 50% of range
  stickDifference = countDownMillis / NUM_PIXELS; //How many millis between redisplay of stick

  //  previousCountDownMillis = millis();

  playDuration = timeCountDown;
  handleStartTimeConvert();

  if (startStopButton.wasPressed()) {

    //    drawDots = false;
    //    greenDisplay.clear();
    //    greenDisplay.writeDisplay();

    beatCounter = 0;
    redDisplay.blinkRate(1);
    redDisplay.writeDisplay();

    // read the adult/child button at the moment we exit this state and use that value to determine which mode runs in the play state
    adultMode = digitalRead(BUTTON_ADULTCHILD);  // 1= Adult, 0= Child
    if (adultMode == 1)
    {
      //      Serial.println("adultMode= ADULT");
    }
    else
    {
      //      Serial.println("adultMode= CHILD");
    }

    GoToNextState();
  }
}


void UpdateWaiting() {

  int currentDistanceValue = bpmPot.getRollingAverage() / smoothingValue;
  checkForDirectionChange(currentDistanceValue);


  if (beatCounter > 1 || startStopButton.wasPressed()) { //at the first compression or push of the Start/Stop button, do all this
    previousDistanceValue = bpmPot.getRollingAverage() / smoothingValue;
    startDistanceValue = previousDistanceValue;
    averageIntervalStartTime = millis();
    overallBpmStartTime = millis();
    averageBpmCounterStart = 0;
    overallBpmCounterStart = beatCounter;
    feedbackMode = LISTENING;
    numCorrections = 0;
    previousDownWasShort = false;
    previousUpWasShort = false;
    numBadDowns = 0;
    numIntervalBeats = 0;
    sentFeedbackLastTime = false;
    redDisplay.clear();
    redDisplay.writeDisplay();
    GoToNextState();
  }
}

void UpdatePlay() {
  int currentDistanceValue = bpmPot.getRollingAverage() / smoothingValue;

  //Hold processing and flash the time display until the first down stroke to
  // make overall BPM reporting more accurate.
  checkForDirectionChange(currentDistanceValue);
  previousDistanceValue = currentDistanceValue;
  decrementCounter(); //Go see if a stickDifference has occurred

  if (decrementNow == true) { //If we counted down to the point to blank a pixel
    strip.clear();
    strip.show();
    isIdle = false;
  }

  if (!isIdle) {
    updateCountdown(ANIMATION_STEP_DELAY); //Go update the stick
    decrementNow = false; // Change to say we're looking for another pixel to blank
  }


  if (numLitPixels > lastLitPixel) { //Reset the pixels to start again from pixel 0 when refreshing the stick.
    numLitPixels = 0;
    isIdle = true;
  }

  redDisplay.blinkRate(0);
  redDisplay.writeDisplay();

  //  if (seconds < 10) {
  //        greenDisplay.writeDigitNum(3, 0);
  //        greenDisplay.writeDisplay();
  //  }

  long currentMillis = millis(); //Record current time (used in calculating what to display on each of the 7-segs)

  handleTimeUpdate(currentMillis);
  //  handleColonBlink(currentMillis);


  if (((millis() - overallBpmStartTime) > (0.75 * playDuration * 1000)) && !sent75pctInfo) {
    commChannel.sendMsg(TIRED, sizeof(TIRED));
    Serial.println("Tired? 75%");
    sent75pctInfo = true;
  }

  if (currentMillis >= (averageIntervalStartTime + AVERAGE_INTERVAL_SAMPLE_TIME)) { //every 5 seconds do every thing below until testing Start/Stop button.
    // do our calculations
    calculateAverageBPM();

    // how is the user doing? Check all three conditions.
    bool isFastEnough = checkPaceProficiencySlow(averageBpm, MIN_ACCEPTABLE_BPM); // is pace fast enough? (i.e., faster than MIN)
    bool isSlowEnough = checkPaceProficiencyFast(averageBpm, MAX_ACCEPTABLE_BPM); // is pace slow enough? (i.e., slower than MAX)
    bool hasGoodDepth = checkDepthProficiency();

    Serial.println((String)averageBpm + " " + (String)isFastEnough  + " " + (String)isSlowEnough);

    // evaluate feedback mode, changing if necessary
    if (feedbackMode == LISTENING && ( !hasGoodDepth || !(isFastEnough == isSlowEnough && isFastEnough) )) { // listening for a mistake. if there is one, kick into correction mode
      if (!hasGoodDepth) {
        feedbackMode = CHECK_FOR_DEPTH;
      }
      if (!isFastEnough) {
        feedbackMode = CHECK_FOR_PACE_SLOW; // if both pace and depth are bad, this line will override the last one, which is what we want.

      }
      if (!isSlowEnough) { //Need to get processing to get here.
        feedbackMode = CHECK_FOR_PACE_FAST;
      }
    }

    // give feedback if appropriate
    if (!sentFeedbackLastTime && feedbackMode != LISTENING) {
      Serial.println("Trying to deliver feedback");
      deliverFeedback(isFastEnough, isSlowEnough, hasGoodDepth);
    }

    sentFeedbackLastTime = !sentFeedbackLastTime;

    // If nothing is bad, reset for next pass and check Start/Stop button.
    // if the user has corrected their mistake, kick back into listening mode

    //This switch statement seems to be for reporting good performance
    switch (feedbackMode) {
      case CHECK_FOR_PACE_SLOW:
        if (isFastEnough) {
          commChannel.sendMsg(RIGHT_SPEED, sizeof(RIGHT_SPEED));
          Serial.println("Was too slow.  Right speed now");
          feedbackMode = LISTENING;
          numCorrections = 0;
        }
        break; //Then move ahead to reset for next pass and check Start/Stop button.
      case CHECK_FOR_PACE_FAST:
        if (isSlowEnough) {
          commChannel.sendMsg(RIGHT_SPEED, sizeof(RIGHT_SPEED));
          Serial.println("Was too fast. Right speed now");
          feedbackMode = LISTENING;
          numCorrections = 0;
        }
        break; //Then move ahead to reset for next pass and check Start/Stop button.
      case CHECK_FOR_DEPTH:
        if (hasGoodDepth) {
          commChannel.sendMsg(GOOD_COMP, sizeof(GOOD_COMP));
          Serial.println("Good compressions");
          feedbackMode = LISTENING;
          numCorrections = 0;
        }
        break; //Then move ahead to reset for next pass and check Start/Stop button.
    }

    // reset for next round
    averageIntervalStartTime = millis();
    numBadDowns = 0;
    numIntervalBeats = 0;
  }



  if (startStopButton.wasPressed() || (timeCountDown + 1 == 0)) {
    overallBpmCount = beatCounter - overallBpmCounterStart;
    overallSeconds = (millis() - overallBpmStartTime) / 1000;
    digitalWrite(LED_AVERAGEBPM, LOW);
    digitalWrite(LED_OVERALLBPM, HIGH);
    redDisplay.print((overallBpmCount * SECS_PER_MINUTE) / overallSeconds);
    //    redDisplay.writeDigitRaw (2, CHR_DOT_4); //Bottom left dot
    redDisplay.writeDisplay();
    strip.clear();
    strip.show();
    commChannel.sendMsg(MED_HELP, sizeof(MED_HELP));
    lastLitPixel = NUM_PIXELS;
    GoToNextState();
  }

}

void UpdateFeedback() {
  //  redDisplay.writeDigitNum(0, FEEDBACK);
  //  redDisplay.writeDisplay();

  beatCounter = 0;

  if (startStopButton.wasPressed()) {
    redDisplay.clear();
    redDisplay.writeDisplay();
    digitalWrite(LED_OVERALLBPM, LOW);
    GoToNextState();
  }
}

void UpdateCalibration() {
  redDisplay.writeDigitNum(0, CALIBRATION);
  //  redDisplay.writeDigitRaw(2, CHR_DOT_3);
  redDisplay.writeDisplay();

  // turn on LEDS to signal the start of the calibration period:
  digitalWrite(LED_AVERAGEBPM, HIGH);
  digitalWrite(LED_OVERALLBPM, HIGH);
  for (int i = 0; i < 24; i = i+2) {
    strip.setPixelColor(i, 0, 100, 0);
  }
  strip.show();
  int calibrateMillis = millis();
  int blinkMillis = millis();

  // calibrate during the first ten seconds
  while (millis() - calibrateMillis < 10000) {
    int sensorValue = analogRead(POT_PIN_BEATSPERMINUTE);


    // record the maximum sensor value
    if (sensorValue > calibrateMaximumDepth) {
      calibrateMaximumDepth = sensorValue;
      Serial.println("Max = " + (String)calibrateMaximumDepth);
    }

    // record the minimum sensor value
    if (sensorValue < calibrateMinimumDepth) {
      calibrateMinimumDepth = sensorValue;
      Serial.println("Min = " + (String)calibrateMinimumDepth);
      Serial.println();
    }

  }

  Serial.println("Time's up!");
  digitalWrite(LED_AVERAGEBPM, LOW);
  digitalWrite(LED_OVERALLBPM, LOW);
  for (int i = 0; i < 24; i = i+2) {
    strip.setPixelColor(i, 0, 0, 0);
  }
  strip.show();
  Serial.println("Final Min = " + (String)calibrateMinimumDepth);
  Serial.println("Final Max = " + (String)calibrateMaximumDepth);
  //  minimumDepth = calibrateMinimumDepth; Check with Emily before implementing
  maximumDepth = calibrateMaximumDepth / smoothingValue;
  delay(5000); //Wait so user stops and doesn't put us into play state.  Cheap way to do it.
  GoToNextState(); //Why does redDisplay show -1 and OverallBpm lights up when we advance?

}
//NEW
//Send some message to redDisplay? "Prss Dn"?
//    maximumDepth = (bpmPot.getRollingAverage() + smoothingValue); //Create a value for maximumDepth a little larger than the actual pot value.
// Used to scale pot distance for inches with map function
//    Serial.println("new maximumDepth = " + (String)maximumDepth);
//new
//Need a way to signal the operator to press, etc.  For now, just hard code it. (273)
//    if (startStopButton.wasPressed()) {
//      greenDisplay.clear();
//      greenDisplay.writeDisplay();  //NEW

//  if (startStopButton.wasPressed()) {




//    }




// the loop function runs over and over again forever
void loop() {

  // always update inputs, no matter what state we're in
  adultChildButton.updateButton();
  //  if (adultChildButton.wasPressed()){
  //    includeCalibration = true; //Should allow us into the Calibration state.
  //    Serial.println("includeCalibration is " +(String)includeCalibration);
  //  }
  startStopButton.updateButton();
  timePot.updatePot();
  bpmPot.updatePot();

  //Serial.println((String)bpmPot.getRollingAverage() + " " + (String)bpmPot.getInstantaneousValue());


  bool error = false;
  switch (currentState) {
    case SETUP:
      UpdateSetup();
      break;
    case WAITING:
      UpdateWaiting();
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

/* Notes from 8/19
    added calibraton code to Calibration state.  Need to finish by using the calibration values.
*/


/*
   Not getting 75% message. Added parens in line 240 and fixed.  Or Rx/Tx jumpers were loose.
  On Util line 24 why add to numCorrections?  To prevent another iteration of this message?

*/

/* Fixed problem where numCorrections was getting reset to 0 and no messages were playing.
    Changed the 'wait for compression' to include a return stament rather than the else statement.  Cleaner.
    Added handling of the recovery from too fast. Was missing.
    Want to fix spurious output on the serial monitor each time a serial command goes to the slave Uno.
*/

/*   First capture calibrate value and set an external condition that allows us to calibrate.
   Use Adult/Child button?
   Signal with Neopixels that we're in the calibrate state.
   Need to check depth of compressions.  Changed to .9 of maxDepth and that seems like it's too little.
   Should smoothingValue increase?
*/

/* Removed code for greenDisplay.  Took out BPM indicator decimals on redDisplay.
    Changed all constant name to UC.  Removed unused variables. Commented out redDisplay count down code.
    May have to look at "Right Speed" message after too fast.  Averaging makes it right speed when it might not be.
*/

/* Fixed bug which caused pixels to start lighting from the place they were if the time didn't
    clock down but the Start/Stop button was pressed.  Now resets properly.  Fixed bug in which we never advanced to
    feedback state.
*/
