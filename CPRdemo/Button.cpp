#include <Arduino.h>

class Button {
    int logicPinNum;
    int ledPinNum;
    bool value;
    bool triggerVal;
    int currentInstantaneousButtonState = 0;
    int prevInstantaneousButtonState = 0;
    int prevRealButtonState = 0;
    unsigned long lastDebounceTime = 0;
    unsigned long debounceDelay = 50;
    
  public:
    Button(int logicPin, int ledPin, bool triggerValue = LOW, long debounce = 50) {
        logicPinNum = logicPin;
        ledPinNum = ledPin;
        triggerVal = triggerValue;
        debounceDelay = debounce;
    }

    // call this to get the value of the button. Note:
    // this only returns TRUE on the loop where the value changed solidly to the trigger value. A held-down button will not return TRUE while it is pressed.
    bool wasPressed() {
      return value;
      }

    // call this function every loop() so that we check the value of the logic pin every time
    void Button::updateButton() {
      // these two lines replace the greenLED/yellowLED functions
       int instReading = digitalRead(logicPinNum);
       digitalWrite(ledPinNum, !instReading);

       // record the current value in an exposed variable (accessible from the outside via wasPressed()
       value = CheckDebounce(instReading);
    }

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
};

 



