/*
    Need to flash the stick until the first down compression
    Call it whenever the time drops to a point that would drop a pixel
    Need to flash the stick interestingly when the count goes to zero to show the user they're done
    Map the time value to the 24 pixels
   Create a variable to set the stick color?
*/

/*Note the current millis and store
   Value from map statement passed in by function call
*/
//update stick with new number of pixels
void updateCountdown(int wait) {
  if (millis() > (startMillis + wait)) { //If we've waited a moment, light the next pixel
    numLitPixels ++;
    strip.setPixelColor(numLitPixels, 0, 0, 100);
    strip.show();
    startMillis = millis(); //Reset for the next stick update
  }
  else return;
}

//  timeCountDown = ((int)map(timePot.getRollingAverage(), 0, 1023, MIN_NUM_SECONDS, MAX_NUM_SECONDS));

void decrementCounter() { //Look to see if we've counted down to the point that a pixel should be blanked and
  //the stick should be updated

  if (millis() > (previousCountDownMillis + stickDifference)) { //If we've counted up to the point that a pixel is to be dropped
    previousCountDownMillis = millis(); //Reset for the next dropped pixel
    decrementNow = true; //It's time to update the stick
  }
  else return;
}

