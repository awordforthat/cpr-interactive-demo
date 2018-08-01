

void calculateAverageBPM() {
  if (millis() >= (averageBpmStartTime + AVERAGE_BPM_SAMPLE_TIME)) {

    int averageBpmCount = (beatCounter - averageBpmCounterStart);
    averageBpm = (averageBpmCount * BPM_CONVERT);

    greenDisplay.print(averageBpm);
    greenDisplay.writeDisplay();
    //Serial.println(averageBpm);

    averageBpmStartTime = millis();
    averageBpmCounterStart = beatCounter;

  }
}


int averageDownDistance = 0;
bool firstTimeThrough = true;

void checkForDirectionChange(int currentDistanceValue) {
  // get the incremental distance that the pot has traveled this loop()
  int deltaDistance = abs(currentDistanceValue - previousDistanceValue);

  // no matter what, record that we've moved either up or down, independent of whether we've changed direction
  if (dirPlus) {

    upDistance += deltaDistance;
  }
  else if (!dirPlus) {

    downDistance += deltaDistance;
  }

  // no matter what, add to total distance traveled
  totalDistance += deltaDistance;


  if ((currentDistanceValue < previousDistanceValue) && !dirPlus) { //Has direction changed?  If so, going up now.
    //    Serial.println("Going up");
    Serial.println("Down stroke length: " + (String)downDistance);
    if (downDistance < (maximumDepth - 2)) {

      // check down stroke length here. Long enough?
      Serial.println("Press deeper"); //This will call an audio file later
      Serial.println();
    }
    //Average of stroke length too shallow?  Rolling buffer Emily prefers

    downDistance = 0;

    //  if (beatCounter % 3 == 0) {
    //        averageDownDistance = downDistance / 3;
    //    if (averageDownDistance < (maximumDepth - 50)) {
    //      Serial.println("Press deeper");
    //      downDistance = 0;
    //  }

    startDistanceValue = currentDistanceValue; //Update start distance
    dirPlus = !dirPlus; //Change direction flag
  }

  if ((currentDistanceValue > previousDistanceValue) && dirPlus ) { //Has direction changed?  If so, going down now.
    directionChangeCounter ++; //Add one to count to obtain cycles.
    //Serial.println("Going down");
    Serial.println("Up stroke length " + (String)upDistance);
    if (upDistance < (maximumDepth - 1)) {

      // check up stroke length here. Long enough?
      Serial.println("Release chest fully"); //This will call an audio file later
      Serial.println();
    }
    //Average of stroke length too shallow?  Rolling buffer Emily prefers
    upDistance = 0;

    startDistanceValue = currentDistanceValue; //Update start distance
    dirPlus = !dirPlus; //Change direction flag
    beatCounter ++;
  }
}


void handleColonBlink(long currentMills) {
  if (currentMillis - previousBlink >= BLINK_INTERVAL) {
    // If a half second has elapsed, do all these things
    previousBlink = currentMillis;
    redDisplay.drawColon(drawDots);
    redDisplay.writeDisplay();
    drawDots = !drawDots; //invert drawDots state
  }
}

void handleTimeUpdate(long currentMillis) {
  if (currentMillis - previousMillis >= interval) {
    // save the last time you updated the display
    previousMillis = currentMillis;

    //If one second has elapsed, do all these things
    // Serial.println("Update display");
    hours = (timeCountDown - (timeCountDown % secsPerHour)) / secsPerHour;
    minutes = ((timeCountDown - (timeCountDown % secsPerMinute) - (hours * secsPerHour))) / secsPerMinute; // secsPerMinute;
    seconds = ((timeCountDown % secsPerHour) % secsPerMinute);
    if (minutes == 0) {
      if (seconds < 10) {
        redDisplay.writeDigitNum(3, 0);
      }
    }

    int displayValue = (minutes * 100) + seconds; //To be pushed to the display.
    redDisplay.print(displayValue);
    redDisplay.writeDisplay();

    timeCountDown--; //Decrement countdown counter
  }
}
