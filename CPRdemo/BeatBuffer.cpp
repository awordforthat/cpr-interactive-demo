#include <Arduino.h>

class BeatBuffer {
    int bufferSize = 100;
    long beatBuffer[100];
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


    int BeatBuffer:: getRollingAverage() {
      long currentTime = millis();
      int numBeats = 0;
      for (int i = 0; i < bufferSize; i++) {
    
        if (beatBuffer[i] != -1) {

          long diff = currentTime - beatBuffer[i];
          if(diff < intervalWidth) {
            numBeats++;
          }

        }
        else {
          break;
        }


      }

      // numBeats/interval width = x / 1000

      float currentBPM = numBeats/intervalWidth * 1000 *60;
      Serial.println(currentBPM);

      return numBeats / this->intervalWidth * 1000 * 60;
    }

    void BeatBuffer::addBeat() {
      beatBuffer[currentIndex] = millis();
      currentIndex++;
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





