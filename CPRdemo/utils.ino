

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

    strokeDistBuffer[halfStrokeCounter] = downDistance;

    // use modulo here to keep the halfStrokeCounter between 0 and 10 (which is the size of the array

    downDistance = 0;


    // I'm arbitrarily choosing to look at beatCounter after the down stroke. We can move it later if you want

    //** choose a number of beats you want to let go by before you check the distance buffer. store that in a variable

    //** if the beatCounter % that variable == 0, perform your check

    //** Your check should be: loop over the strokeDistBuffer using a for loop. In this case, you will need to increment the counter by 2, not one.
    //** e.g. for(int i = 0; i < 10; i += 2)

    //** sum those values

    //** divide by *half* the number of elements in the array (because we only are interested in one direction, the up or the down

    //** that is your rolling average for that stroke direction. Print it out to the serial monitor


    startDistanceValue = currentDistanceValue; //Update start distance
    dirPlus = !dirPlus; //Change direction flag
    halfStrokeCounter++;
    halfStrokeCounter = halfStrokeCounter % 10; // 10 should be variable...later
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
    strokeDistBuffer[halfStrokeCounter] = upDistance;

    upDistance = 0;

    startDistanceValue = currentDistanceValue; //Update start distance
    dirPlus = !dirPlus; //Change direction flag
    beatCounter ++;
    halfStrokeCounter++;
    halfStrokeCounter = halfStrokeCounter % 10;
  }

  // cool. bit of an off-by-one error since I think we register a stroke when we enter the state 
  // even if the pot isn't moving. Not worried about that yet.

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

//  if (beatCounter % 3 == 0) {
//        averageDownDistance = downDistance / 3;
//    if (averageDownDistance < (maximumDepth - 50)) {
//      Serial.println("Press deeper");
//      downDistance = 0;
//  }
