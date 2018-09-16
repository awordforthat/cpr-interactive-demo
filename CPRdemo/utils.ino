

void calculateAverageBPM() {
  int averageBpmCount = (beatCounter - averageBpmCounterStart);
  averageBpm = (averageBpmCount * BPM_CONVERT);

  // print to screen
  redDisplay.print(averageBpm);
//  redDisplay.writeDigitRaw (2, CHR_DOT_3); //Top left dot
  redDisplay.writeDisplay();
  digitalWrite(LED_AVERAGEBPM, HIGH);


  averageBpmCounterStart = beatCounter;
}

void handleStartTimeConvert() {
//  hours = (timeCountDown - (timeCountDown % SECS_PER_HOUR)) / SECS_PER_HOUR;
  minutes = ((timeCountDown - (timeCountDown % SECS_PER_MINUTE) - (hours * SECS_PER_HOUR))) / SECS_PER_MINUTE; // SECS_PER_MINUTE;
  seconds = ((timeCountDown % SECS_PER_HOUR) % SECS_PER_MINUTE);

  if (minutes == 0) {
    if (seconds < 10) {
      redDisplay.writeDigitNum(3, 0);
    }
  }

  int displayValue = (minutes * 100) + seconds; //To be pushed to the display.


  redDisplay.print(displayValue);
  //  redDisplay.drawColon(true); // must go after print to display
  redDisplay.writeDisplay();
}

boolean checkPaceProficiencySlow(int averageBpm, int lowLimit) { //values from averageBpm and MIN_ACCEPTABLE_BPM
  return averageBpm > lowLimit;
  //Returns 'true' for isFastEnough if averageBPM is greater than MIN_ACCEPTABLE_BPM
  //Returns 'false' if averageBPM is less than MIN_ACCEPTABLE_BPM
}

boolean checkPaceProficiencyFast(int averageBpm, int highLimit) { //values from averageBpm and MAX_ACCEPTABLE_BPM
  return averageBpm < highLimit;
  //Returns 'true' for isSlowEnough if averageBPM is less than MIN_ACCEPTABLE_BPM
  //Returns 'false' if averageBPM is greater than MIN_ACCEPTABLE_BPM
}

bool checkDepthProficiency() {
  // how many beats did we expect during this interval?

  //  Serial.println("Got " + (String)numIntervalBeats + " this round");
  //  Serial.println("Of those, " + (String)numBadDowns + " were short");

  return (numBadDowns / numIntervalBeats) < 0.2;
}

//This switch statement seems to be for handling feedback for poor performance
void deliverFeedback(bool goingFastEnough, boolean goingSlowEnough, bool hasGoodDepth) {
  Serial.println("Delivering feedback for mode " + (String)feedbackMode);
  switch (feedbackMode) {
    case LISTENING:
      if (numCorrections == 0)
      {
        commChannel.sendMsg(GOT_THIS, sizeof(GOT_THIS));
        Serial.println("Got this Utils");
        Serial.println();
        numCorrections++;
      }

      break;
    case CHECK_FOR_PACE_SLOW:
      if (!goingFastEnough) {
        deliverPaceFeedback(true); // speed up!
      }
      break;

    case CHECK_FOR_PACE_FAST:
      if (!goingSlowEnough) {
        Serial.println("Inside deliverFeedback - toooo faaaast!");
        deliverPaceFeedback(false);  // slow down!
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

void deliverPaceFeedback(bool shouldSpeedUp) {

  //  Serial.println("Number of corrections = " + (String)numCorrections);
  if (numCorrections < MAX_NUM_CORRECTIONS) {

    if (shouldSpeedUp) { // too slow! go faster
      if (numCorrections < 1 ) {
        commChannel.sendMsg(LITTLE_FASTER, sizeof(LITTLE_FASTER));
        Serial.println("Little faster");
        Serial.println();
      }

      else {
        commChannel.sendMsg(LITTLE_FASTER_AND_MUSIC, sizeof(LITTLE_FASTER_AND_MUSIC));
        Serial.println("Play Little Faster and music");
        Serial.println();
      }
    }

    else { // too fast! go a little slower

      if (numCorrections < 1 ) {
        commChannel.sendMsg(LITTLE_SLOWER, sizeof(LITTLE_SLOWER)); //To be changed to a 'slower' message.
        Serial.println("Little slower");
        Serial.println();
      }

      else {
        commChannel.sendMsg(LITTLE_SLOWER_AND_MUSIC, sizeof(LITTLE_SLOWER_AND_MUSIC));
        Serial.println("Play Little Slower and music");
        Serial.println();
      }
    }

    numCorrections++;
  }

  //

}

void deliverDepthFeedback() {
  Serial.println("Num corrections: " + (String)numCorrections);
  if (numCorrections < MAX_NUM_CORRECTIONS) {
    commChannel.sendMsg(PUSH_HARDER, sizeof(PUSH_HARDER));
    numCorrections++;
    Serial.println("Push harder!");
    Serial.println();
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


    if (downDistance < maximumDepth * 0.9 && downDistance != 1) {
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
    Serial.println("Going down!");

    if (upDistance < maximumDepth * 0.90 && upDistance != 1) {
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
