

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

void checkForDirectionChange(int currentDistanceValue) {

  if ((currentDistanceValue < previousDistanceValue) && !dirPlus) { //Has direction changed?  If so, going up now.
    //    Serial.println("Going up");

    upDistance += abs(startDistanceValue - previousDistanceValue);
    Serial.println("upDistance: " + (String)upDistance);

    totalDistance += upDistance;
    Serial.println("totalDistance: " + (String)totalDistance);
    Serial.println();

    startDistanceValue = currentDistanceValue; //Update start distance
    dirPlus = !dirPlus; //Change direction flag
    //  if (beatCounter % 3 == 0) {
    //        averageDownDistance = downDistance / 3;
    //    if (averageDownDistance < (maximumDepth - 50)) {
    //      Serial.println("Press deeper");
    //      downDistance = 0;
    //  }
  }

  if ((currentDistanceValue > previousDistanceValue) && dirPlus ) { //Has direction changed?  If so, going down now.
    directionChangeCounter ++; //Add one to count to obtain cycles.
    Serial.println("Going down");

    downDistance += abs(startDistanceValue - previousDistanceValue);
    Serial.println("downDistance: " + (String)downDistance);

    totalDistance += downDistance;
    Serial.println("totalDistance: " + (String)totalDistance);
    Serial.println();

    startDistanceValue = currentDistanceValue; //Update start distance
    dirPlus = !dirPlus; //Change direction flag
    beatCounter ++;
  }
}
// TODO: calculate average distance compressed. Rolling and gross
//Measure every compression both ways - two variables to store most recent N strokes. Reset after each evaluation
//Every few compressions verify good depth and good return - use modulo on beatCounter to determine when to evaluate
//Accumulate total depth and total beats to report average depth at the end - this is a third variable that is the sum of all the compressions both ways






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
