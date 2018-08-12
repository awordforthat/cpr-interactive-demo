

#include <RS485_non_blocking.h>
#include <WaveHC.h>
#include <WaveUtil.h>

SdReader card;    // This object holds the information for the card
FatVolume vol;    // This holds the information for the partition on the card
FatReader root;   // This holds the information for the volumes root directory
FatReader file;   // This object represent the WAV file
WaveHC wave;      // This is the only wave (audio) object, since we will only play one at a time

const byte ONE [] = "ONE";
const byte TWO [] = "TWO";
const byte THREE [] = "THREE";
const byte FOUR [] = "FOUR";
const byte FIVE [] = "FIVE";
const byte SIX [] = "SIX";
const byte SEVEN [] = "SEVEN";
const byte EIGHT [] = "EIGHT";
const byte NINE [] = "NINE";
const byte TEN [] = "TEN";
const int FILE_COUNT = 9; //Will be 10 once Stayin' Alive is ripped.
/*
   Define macro to put error messages in flash memory
*/
#define error(msg) error_P(PSTR(msg))

int fAvailable ()
{
  return Serial.available ();
}

int fRead ()
{
  return Serial.read ();
}


RS485 commChannel (fRead, fAvailable, NULL, 20);

void setup ()
{
  Serial.begin (9600);
  commChannel.begin ();

  // initialize audio stuff
  // try card.init(true) if errors occur on V1.0 Wave Shield
  if (!card.init()) {
    error("Card init. failed!");
  }
  // enable optimize read - some cards may timeout
  card.partialBlockRead(true);

  if (!vol.init(card)) {
    error("No partition!");
  }
  if (!root.openRoot(vol)) {
    error("Couldn't open root"); return;
  }
  putstring_nl("Files found:");
  root.ls();
}  // end of setup

// forward declarition
void playcomplete(FatReader &file);
void loop ()
{
  if (commChannel.update ())

  {
    String msg = commChannel.getData();
    
    if(msg == ONE) {
      playcomplete("GoodCmpA.wav");
    }
     if(msg == TWO) {
      playcomplete("GotThsA.wav");
    }
     if(msg == THREE) {
      playcomplete("LtlFstr.wav");
    }
     if(msg == FOUR) {
      playcomplete("MedHelpA.wav");
    }
         if(msg == FIVE) {
      playcomplete("MuscPcA.wav");
    }
         if(msg == SIX) {
      playcomplete("PshHrdrA.wav");
    }
         if(msg == SEVEN) {
      playcomplete("RTSpdA.wav");
    }
         if(msg == EIGHT) {
      playcomplete("TiredA.wav");
    }
         if(msg == NINE) {
      playcomplete("RTSpdA.wav");
    }
         if(msg == TEN) {
      playcomplete("RTSpdA.wav");
    }
    Serial.write (commChannel.getData (), commChannel.getLength ());
    Serial.println ();
  }
}  // end of loop



/////////////////////////////////// HELPERS
/*
   print error message and halt
*/
void error_P(const char *str) {
  PgmPrint("Error: ");
  SerialPrint_P(str);
  sdErrorCheck();
  while (1);
}

/*
   print error message and halt if SD I/O error, great for debugging!
*/
void sdErrorCheck(void) {
  if (!card.errorCode()) return;
  PgmPrint("\r\nSD I/O error: ");
  Serial.print(card.errorCode(), HEX);
  PgmPrint(", ");
  Serial.println(card.errorData(), HEX);
  while (1);
}

// Plays a full file from beginning to end with no pause.
void playcomplete(char *name) {
  // call our helper to find and play this name
  playfile(name);
  while (wave.isplaying) {
  // do nothing while its playing
  }
  // now its done playing
}

void playfile(char *name) {
  // see if the wave object is currently doing something
  if (wave.isplaying) {// already playing something, so stop it!
    wave.stop(); // stop it
  }
  // look in the root directory and open the file
  if (!file.open(root, name)) {
    putstring("Couldn't open file "); Serial.print(name); return;
  }
  // OK read the file and turn it into a wave object
  if (!wave.create(file)) {
    putstring_nl("Not a valid WAV"); return;
  }
  
  // ok time to play! start playback
  wave.play();
}
