#include <Arduino.h>

class Button {
    int pinNum;
    bool value;
    bool triggerVal;
    int currentInstantaneousButtonState = 0;
    int prevInstantaneousButtonState = 0;
    int prevRealButtonState = 0;
    unsigned long lastDebounceTime = 0;
    unsigned long debounceDelay = 50;
    
  public:
    Button(int pin, bool triggerValue = LOW, long debounce = 50) {
        pinNum = pin;
        triggerVal = triggerValue;
        debounceDelay = debounce;
    }
    
    bool wasPressed() {return value;}
    
    void Button::updateButton() {
       value = CheckDebounce(digitalRead(pinNum));
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

 



