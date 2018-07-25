#include <Arduino.h>

class Potentiometer {
    int logicPinNum;
    int bufferSize;
    int hysteresisBuffer[100]; // max size 100; by changing bufferSize you can change the effective size to something smaller
    int hysteresisIndex;
    bool value;

    
  public:
    Potentiometer(const uint8_t logicPin, int bufferSize) {
        logicPinNum = logicPin;
        hysteresisBuffer[bufferSize] = {};
        hysteresisIndex = 0;
        this->bufferSize = bufferSize;
    }

    // Call this sometime before getRollingAverage() to fill the buffer and get a starting average value. 
    // This will minimize large jumps in the average when you start to read the pot values
    void Potentiometer::init() {
      for(int i = 0; i < bufferSize; i++) {
        hysteresisBuffer[i] = analogRead(logicPinNum);
      }
    }

    // Returns the average of the past N values, where N is the size of the rolling buffer
    float getRollingAverage() {
      float average;
      for(int i = 0; i < bufferSize; i++) {
        average += hysteresisBuffer[i];
      }
      return average / bufferSize;
    }

    // Returns the most recently read raw value
    float getInstantaneousValue() {
      return hysteresisBuffer[hysteresisIndex];
    }


    // call this function every loop() so that we check the value of the logic pin every time
    void Potentiometer::updatePot() {
       hysteresisBuffer[hysteresisIndex] = (int)analogRead(logicPinNum);
       hysteresisIndex =(hysteresisIndex + 1)  % bufferSize;  
    }

};

 



