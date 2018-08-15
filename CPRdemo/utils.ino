

void calculateAverageBPM() {
  int averageBpmCount = (beatCounter - averageBpmCounterStart);
  averageBpm = (averageBpmCount * BPM_CONVERT);

  // print to screen
  greenDisplay.print(averageBpm);
  greenDisplay.writeDigitRaw (2, chrDot3); //Top left dot
  greenDisplay.writeDisplay();
  digitalWrite(LED_AVERAGEBPM, HIGH);


  averageBpmCounterStart = beatCounter;
}

boolean checkPaceProficiency(int averageBpm, int lowLimit) {
  return averageBpm > lowLimit;
}

bool checkDepthProficiency() {
  // how many beats did we expect during this interval?

//  Serial.println("Got " + (String)numIntervalBeats + " this round");
//  Serial.println("Of those, " + (String)numBadDowns + " were short");

  return (numBadDowns / numIntervalBeats) < 0.15;
}

void deliverFeedback(bool hasGoodPace, bool hasGoodDepth) {
  switch (feedbackMode) {
    case LISTENING:
      if (numCorrections == 0)
      {
        commChannel.sendMsg(GOT_THIS, sizeof(GOT_THIS));
        numCorrections++;
      }

      break;
    case CHECK_FOR_PACE:
      if (!hasGoodPace) {
        deliverPaceFeedback();
      }
      break;
    case CHECK_FOR_DEPTH:
      if (!hasGoodDepth) {
        deliverDepthFeedback();
      }
      break;
    default:
      Serial.println("Feedback mode " + (String) + " was not recognized!");
  }
}

void deliverPaceFeedback() {

  if (numCorrections < MAX_NUM_CORRECTIONS) {

    if (numCorrections < 1) {
      commChannel.sendMsg(LITTLE_FASTER, sizeof(LITTLE_FASTER));
      Serial.println("Little faster");
    }
    else {
      commChannel.sendMsg(INTRO_AND_MUSIC, sizeof(INTRO_AND_MUSIC));
      Serial.println("Play music");
    }
    numCorrections++;
  }
}

void deliverDepthFeedback() {
  if (numCorrections < MAX_NUM_CORRECTIONS) {
    Serial.println("Do better at depth!");
  }
}



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

  // protect against jitter: a stroke must have a minimum distance before we allow it to count as a real stroke
  if((dirPlus && upDistance < MIN_STROKE_DISTANCE) || (!dirPlus && downDistance < MIN_STROKE_DISTANCE)) {
    //Serial.println("Jitter!");
    return;
  }


  if ((currentDistanceValue < previousDistanceValue) && !dirPlus) { //Has direction changed?  If so, going up now.
    //    Serial.println("Going up");

    if(downDistance < maximumDepth * 0.75) {
      Serial.println("Short down!");
      previousDownWasShort = true;
      numBadDowns++;
    }
    else {
      previousDownWasShort = false;
    }

    downDistance = 0;

    startDistanceValue = currentDistanceValue; //Update start distance
    dirPlus = !dirPlus; //Change direction flag

  }

  if ((currentDistanceValue > previousDistanceValue) && dirPlus ) { //Has direction changed?  If so, going down now.
    directionChangeCounter ++; //Add one to count to obtain cycles.
    //    Serial.println("Going down");

    if(upDistance < maximumDepth * 0.75) {
      // Serial.println("Short up!");
      previousUpWasShort = true;
    }
    else {
      previousUpWasShort = false;
    }
    upDistance = 0;


    startDistanceValue = currentDistanceValue; //Update start distance
    dirPlus = !dirPlus; //Change direction flag

    beatCounter++;
    numIntervalBeats++;



  }


}



void handleColonBlink(long currentMillis) {
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

void handleStartTimeConvert() {
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
  redDisplay.drawColon(true); // must go after print to display
  redDisplay.writeDisplay();
}

//  if (beatCounter % 3 == 0) {
//        averageDownDistance = downDistance / 3;
//    if (averageDownDistance < (maximumDepth - 50)) {
//      Serial.println("Press deeper");
//      downDistance = 0;
//  }
