#include <Arduino.h>

class BeatBuffer {
    int bufferSize = 40;
    long beatBuffer[40];
    int currentIndex;
    float intervalWidth; // in milliseconds, how long the rolling average window is



  public:
    BeatBuffer( int intervalWindow) {
      this->beatBuffer[bufferSize] = {};
      currentIndex = 0;
      intervalWidth = 10000;

      for (int i = 0; i < bufferSize; i++) {
        beatBuffer[i] = -1;
      }
    }


// returns the current BPM based on the number of beats that were received in the most
// recent N seconds
    int BeatBuffer:: getRollingAverage() {
      long currentTime = millis();
      float numBeats = 0;
      long localMin = 10000000; //hack - just want something very large
      long localMax = 0;
      for (int i = 0; i < bufferSize; i++) {
        if (beatBuffer[i] != -1) {                      // this line ignores values in the buffer that are left over from initialization
          long diff = currentTime - beatBuffer[i];      // how long ago was the beat recorded?
          if (diff < intervalWidth) {                   // if it was within the time frame, count it and do some other stuff
            numBeats++;
            if (beatBuffer[i] < localMin) {             // within the interval, record the earliest beat
              localMin = beatBuffer[i];
            }

            if (beatBuffer[i] > localMax) {             // and also the most recent
              localMax = beatBuffer[i];
            }
          }

        }

      }
      long duration = localMax - localMin;              // calculate the actual duration of the interval from earliest to latest beat
      return (float)numBeats / duration * 60 * 1000;    // send back calculated BPM
    }

//  a beat is recorded as the timestamp of when it occurred
    void BeatBuffer::addBeat() {
      beatBuffer[currentIndex] = millis();
      currentIndex = (currentIndex + 1) % bufferSize;
    }

    void BeatBuffer::printBuffer () {
      Serial.print("[");
      for (int i = 0; i < bufferSize; i++) {
        Serial.print(beatBuffer[i]);
        if (i != bufferSize) {
          Serial.print(", ");
        }
      }
      Serial.println("]");
    }

    void BeatBuffer::reset() {
      for (int i = 0; i < bufferSize; i++) {
        beatBuffer[i] = -1;
      }
      currentIndex = 0;
    }

};





