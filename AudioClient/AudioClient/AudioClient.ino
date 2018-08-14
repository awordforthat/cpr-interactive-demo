

#include <RS485_non_blocking.h>
#include <WaveHC.h>
#include <WaveUtil.h>

SdReader card;    // This object holds the information for the card
FatVolume vol;    // This holds the information for the partition on the card
FatReader root;   // This holds the information for the volumes root directory
FatReader file;   // This object represent the WAV file
WaveHC wave;      // This is the only wave (audio) object, since we will only play one at a time


const byte GOOD_COMP [] = "ONE";
const byte RIGHT_SPEED [] = "TWO";
const byte GOT_THIS [] = "THREE";
const byte LITTLE_FASTER [] = "FOUR";
const byte INTRO_AND_MUSIC [] = "FIVE";
const byte PUSH_HARDER [] = "SIX";
const byte MED_HELP [] = "SEVEN";
const byte TIRED [] = "EIGHT";
const byte MUSIC_ONLY [] = "NINE";
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

// forward declaration
void playcomplete(FatReader &file);
void loop ()
{
  if (commChannel.update ())

  {
    String msg = commChannel.getData();
    Serial.println(msg);

    if (msg == GOOD_COMP) {
      playcomplete("GoodCmpA.wav"); // good comp (but not good pace??)
    }
    if (msg == RIGHT_SPEED) {
      playcomplete("RTSpdA.wav");  // good pace (but shallow?)
    }
    if (msg == GOT_THIS) {
      playcomplete("GotThsA.wav"); // good comp && good pace
    }
    if (msg == LITTLE_FASTER) {
      playcomplete("LtlFstrA.wav");  // too slow
    }
    if (msg == INTRO_AND_MUSIC) {
      playcomplete("MuscPcA.wav");  // too slow (maybe 2nd time)
      playfile("StynAlvC.wav");
    }
    if (msg == PUSH_HARDER) {
      playcomplete("PshHrdrA.wav");  // too shallow
    }
    if (msg == MED_HELP) {
      playcomplete("MedHelpA.wav"); // ending (feedback)
    }
    if (msg == TIRED) {
      playcomplete("TiredA.wav"); // 75% through duration of play
    }
    if (msg == MUSIC_ONLY) {
      playfile("StynAlvA.wav"); // music only, no intro
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
