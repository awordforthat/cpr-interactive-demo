

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

boolean checkPaceProficiencySlow(int averageBpm, int lowLimit) {
  return averageBpm > lowLimit;
}

boolean checkPaceProficiencyFast(int averageBpm, int highLimit) {
  return averageBpm < highLimit;
}

bool checkDepthProficiency() {
  // how many beats did we expect during this interval?

  //  Serial.println("Got " + (String)numIntervalBeats + " this round");
  //  Serial.println("Of those, " + (String)numBadDowns + " were short");

  return (numBadDowns / numIntervalBeats) < 0.2;
}

void deliverFeedback(bool goingFastEnough, boolean goingSlowEnough, bool hasGoodDepth) {
  switch (feedbackMode) {
    case LISTENING:
      if (numCorrections == 0)
      {
        commChannel.sendMsg(GOT_THIS, sizeof(GOT_THIS));
        numCorrections++;
      }

      break;
    case CHECK_FOR_PACE_SLOW:
      if (!goingFastEnough) {
        deliverPaceFeedback( true); // speed up!
      }
      break;
    case CHECK_FOR_PACE_FAST:
      if (!goingSlowEnough) {
        deliverPaceFeedback( false);  // slow down!
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

void deliverPaceFeedback( bool shouldSpeedUp) {

  if (numCorrections < MAX_NUM_CORRECTIONS) {

    if (shouldSpeedUp) { // too slow! go faster
      if (numCorrections < 1 ) {
        commChannel.sendMsg(LITTLE_FASTER, sizeof(LITTLE_FASTER));
        Serial.println("Little faster");
      }
      else {
        commChannel.sendMsg(INTRO_AND_MUSIC, sizeof(INTRO_AND_MUSIC));
        Serial.println("Play music");
      }
    }
    else { // too fast! go a little slower
      if (numCorrections < 1 ) {
        commChannel.sendMsg(TIRED, sizeof(TIRED));
        Serial.println("Little slower");
      }
      else {
        commChannel.sendMsg(INTRO_AND_MUSIC, sizeof(INTRO_AND_MUSIC));
        Serial.println("Play music");
      }
    }

    numCorrections++;
  }
}

void deliverDepthFeedback() {
  Serial.println("Num corrections: " + (String)numCorrections);
  if (numCorrections < MAX_NUM_CORRECTIONS) {
    commChannel.sendMsg(PUSH_HARDER, sizeof(PUSH_HARDER));
    numCorrections++;
    Serial.println("Push harder!");
  }
}



void checkForDirectionChange(int currentDistanceValue) {

  // get the incremental distance that the pot has traveled this loop()
  int deltaDistance = abs(currentDistanceValue - previousDistanceValue);


  // no matter what, record that we've moved either up or down, independent of whether we've changed direction
  if (dirPlus) {

    upDistance += deltaDistance;
    //        Serial.println("deltaDistance " + (String)deltaDistance);
    //        Serial.println("upDistance is " + (String)upDistance);
  }
  else if (!dirPlus) {

    downDistance += deltaDistance;
    //    Serial.println("deltaDistance " + (String)deltaDistance);
    //    Serial.println("downDistance is " + (String)downDistance);
  }


  // no matter what, add to total distance traveled
  totalDistance += deltaDistance;

  // protect against jitter: a stroke must have a minimum distance before we allow it to count as a real stroke
  if ((dirPlus && upDistance < MIN_STROKE_DISTANCE) || (!dirPlus && downDistance < MIN_STROKE_DISTANCE)) {
    //Serial.println("Jitter!");
    return;
  }

  bool spuriousRead = false;
  if ((currentDistanceValue < previousDistanceValue) && dirPlus) {
//    Serial.println("Going up!");

    
    if (downDistance < maximumDepth * 0.6 && downDistance != 1) {
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
  else if ((currentDistanceValue > previousDistanceValue) && !dirPlus) {
//    Serial.println("Going down!");

    if (upDistance < maximumDepth * 0.60 && upDistance != 1) {
      shortUpStrokeCounter++;
      previousUpWasShort = true;
    }
    else {
      previousUpWasShort = false;
    }

    upDistance = 0;
    startDistanceValue = currentDistanceValue;
    dirPlus = !dirPlus;
    beatCounter++;
    numIntervalBeats++;
  }

  previousDistanceValue = currentDistanceValue;


}




//  if (beatCounter % 3 == 0) {
//        averageDownDistance = downDistance / 3;
//    if (averageDownDistance < (maximumDepth - 50)) {
//      Serial.println("Press deeper");
//      downDistance = 0;
//  }
