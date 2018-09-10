/* Create a way to take time between steps that doesn't use delay
    Need to flash the stick until the first down compression
    Need a function to update the stick by sweeping the pixels
    Call it every 5 seconds or so or at the same time as we post the BPMs
    Need to flash the stick interestingly when the count goes to zero to show the user they're done
    Map the time value to the 24 pixels
   Create a variable to set the stick color?

*/

/*Note the current millis and store
   Loop the number of segments to be lit. Value from map statement passed in by function call
   Loop until x millis have passed (to set the time between each pixel to illuminate)
   Write the stick with next pixel

   Reset millis
   }
   }
*/

void updateCountdown(int wait) {
  //  strip.clear();
  //  strip.show();
  //  Serial.println("Cleared the strip");

  if (millis() > (startMillis + wait)) {

    numLitPixels ++;
    strip.setPixelColor(numLitPixels, 0, 0, 100);
    strip.show();
    startMillis = millis();
    Serial.println(millis());
  }
  else return;
}


