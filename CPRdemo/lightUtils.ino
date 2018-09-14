//update stick with new number of pixels
void updateCountdown(int wait) {
  if (millis() > (startMillis + wait)) { //If we've waited a moment, light the next pixel
    strip.setPixelColor(numLitPixels, 0, 0, 10);
    strip.show();
    numLitPixels ++;
    startMillis = millis(); //Reset for the next stick update
  }
  else return;
}

//  timeCountDown = ((int)map(timePot.getRollingAverage(), 0, 1023, MIN_NUM_SECONDS, MAX_NUM_SECONDS));

void decrementCounter() { //Look to see if we've counted down to the point that a pixel should be blanked and
  //the stick should be updated

  if (millis() > (previousCountDownMillis + stickDifference)) { //If we've counted up to the point that a pixel is to be dropped
    previousCountDownMillis = millis(); //Reset for the next dropped pixel
    lastLitPixel --;
    decrementNow = true; //It's time to update the stick
//    Serial.println("We've decremented!");
//    Serial.println("lastLitPixel is now: " + (String)lastLitPixel);
    if (lastLitPixel < 0) {
      lastLitPixel = NUM_PIXELS; //Reset for next user
    }
  }
  else return;
}

