/*Upload this sketch to the Arduino used to run the CPRdemo sketch
   Open the serial monitor and check the display
*/
#include <Wire.h> // Enable this line if using Arduino Uno, Mega, etc.
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#define POT_PIN_TIME A0 // Input from TIME pot
#define POT_PIN_BEATSPERMINUTE A3 // Input from BEATSPERMINUTE pot
#define LED_AVERAGEBPM 9 // Set pin 8 for Start/Stop button LED
#define LED_OVERALLBPM 10 //Set pin 12 for Adult/Child button LED
#define NUM_BPM_SAMPLES 40

Adafruit_7segment displayTime = Adafruit_7segment();// Add time display
Adafruit_7segment displayBeatsPerMinute = Adafruit_7segment(); // Add BeatsPerMinute display
int buttonStartStop = 2; //Set pin 2 for Start/Stop button
//int buttonStepState = 5; //Set pin 5 for StepState button
int buttonAdultChild = 4; //Set pin 4 for Adult/Child button
int ledStartStop = 8; // Set pin 8 for Start/Stop button LED
int ledAdultChild = 12; //Set pin 12 for Adult/Child button LEDs
int loopCount = 0;
void setup() {

  pinMode(buttonStartStop, INPUT_PULLUP); //button is active low
  //  pinMode(buttonStepState,INPUT_PULLUP); //button is active low
  pinMode(buttonAdultChild, INPUT_PULLUP); //button is in Child mode when pressed (low); Adult mode when released (high).
  pinMode(ledStartStop, OUTPUT);
  pinMode(ledAdultChild, OUTPUT);
  pinMode(LED_AVERAGEBPM, OUTPUT);
  pinMode(LED_OVERALLBPM, OUTPUT);
  Serial.begin(9600);
  displayTime.begin(0x71);
  displayBeatsPerMinute.begin(0x70);
}

void loop() {
  if (loopCount <= 1) {
    for (int x = 0; x <= 1; x++) {

      Serial.println("Start/Stop LED will blink in one second.");
      Serial.println();
      delay(1000);

      for (int x = 0; x < 4; x++) { //Loop to test button LEDs.  Turn them on and off sequentially.
        digitalWrite(ledStartStop, HIGH);
        Serial.println ("Start/Stop LED on");
        delay (500);

        digitalWrite(ledStartStop, LOW);
        Serial.println ("Start/Stop LED off");
        Serial.println ();
        delay (500);
      }

      Serial.println("Adult/Child LED will blink in one second.");
      Serial.println();
      delay(1000);

      for (int x = 0; x < 4; x++) { //Loop to test button LEDs.  Turn them on and off sequentially.
        digitalWrite(ledAdultChild, HIGH);
        Serial.println ("Adult/Child LED on");
        delay (500);

        digitalWrite(ledAdultChild, LOW);
        Serial.println ("Adult/Child LED off") ;
        Serial.println ();
        delay (500);
      }

      delay (1000); //Hold after completion of for loop

      Serial.println("LED_AVERAGEBPM will blink in one second.");
      Serial.println();
      delay(1000);

      for (int x = 0; x < 4; x++) { //Loop to test button LEDs.  Turn them on and off sequentially.
        digitalWrite(LED_AVERAGEBPM, HIGH);
        Serial.println ("LED_AVERAGEBPM LED on");
        delay (500);

        digitalWrite(LED_AVERAGEBPM, LOW);
        Serial.println ("LED_AVERAGEBPM LED off");
        Serial.println ();
        delay (500);
      }

      Serial.println("LED_OVERALLBPM will blink in one second.");
      Serial.println();
      delay(1000);

      for (int x = 0; x < 4; x++) { //Loop to test button LEDs.  Turn them on and off sequentially.
        digitalWrite(LED_OVERALLBPM, HIGH);
        Serial.println ("LED_OVERALLBPM LED on");
        delay (500);

        digitalWrite(LED_OVERALLBPM, LOW);
        Serial.println ("LED_OVERALLBPM LED off") ;
        Serial.println ();
        delay (500);
      }

      delay (1000); //Hold after completion of for loop


      //Test the green Start/Stop button
      int buttonStartStopVal = 0;
      Serial.println ("Push the green Start/Stop button and check for changing value below.") ;
      Serial.println();

      for (int x = 0; x < 100; x++) {
        buttonStartStopVal = digitalRead(buttonStartStop);
        buttonStartStopVal = !buttonStartStopVal; //Invert button state to make turn LED on when button is depressed

        digitalWrite(ledStartStop, buttonStartStopVal);
        Serial.print ("Start/Stop button value is: ");
        Serial.println (buttonStartStopVal);

      }

      buttonStartStopVal = 0; //Turn off ledStartStop
      digitalWrite(ledStartStop, buttonStartStopVal);
      Serial.println (buttonStartStopVal);
      delay (1000);

      //Test the yellow Adult/Child button
      int buttonAdultChildVal = 0;
      Serial.println ("Repeatedly press the yellow Adult/Child button and check for changing value below") ;
      for (int x = 0; x < 100; x++) {

        buttonAdultChildVal = digitalRead(buttonAdultChild);
        buttonAdultChildVal = !buttonAdultChildVal; //Invert button state to make turn LED on when button is depressed

        digitalWrite(ledAdultChild, buttonAdultChildVal);
        Serial.print ("Adult/Child button value is: ");
        Serial.println (buttonAdultChildVal);

      }

      buttonAdultChildVal = 0;
      digitalWrite(ledAdultChild, buttonAdultChildVal);
      Serial.println (buttonAdultChildVal); //Turn off ledAdultChild
      delay (1000);

      //    //Test the StepState button
      //    int buttonStepStateVal = 0;
      //    Serial.println ("Repeatedly press the StepState button now") ;
      //    for (int x=0; x < 1000; x++) {
      //
      //      buttonStepStateVal = digitalRead(buttonStepState);
      //      buttonStepStateVal = !buttonStepStateVal; //Invert button state to make turn LED on when button is depressed
      //
      //      digitalWrite(ledAdultChild, buttonStepStateVal);
      //      Serial.print ("StepState button value is: ");
      //      Serial.println (buttonStepStateVal);
      //
      //    }
      //
      //Check time pot and display.  Print to serial and time display
      Serial.println ("Rotate the Time pot and check the red display for changing values.") ;
      Serial.println();
      delay(1000);
      for (int x = 0; x < 50; x++) {
        int timePotValue = analogRead(POT_PIN_TIME);
        Serial.print("Time pot value is: ");
        Serial.print(timePotValue);
        Serial.println();
        displayTime.print(timePotValue);
        displayTime.writeDisplay();
        delay(100);
      }
      delay(1000);

      //Check beatsPerMinute pot and display.  Print to serial and time display.
      Serial.println ("Slide the Beats Per Minute pot and check the green display for changing values.") ;
      Serial.println();
      delay(1000);
      for (int x = 0; x < 50; x++) {

        int beatsPerMinutePotValue = analogRead(POT_PIN_BEATSPERMINUTE);
        Serial.print("Beats Per Minute pot value is: ");
        Serial.print(beatsPerMinutePotValue);
        Serial.println();
        displayBeatsPerMinute.print(beatsPerMinutePotValue);
        displayBeatsPerMinute.writeDisplay();

        delay(100);
      }
      Serial.println("If everything checked out, upload the CPRdemo sketch.");
      Serial.println();

      loopCount ++;
      if (loopCount == 1) {
        Serial.println("This sketch will repeat one more time.");
        Serial.println();
      }
      delay(5000); //Wait 5 seconds before running this again.
    }
  }
}
