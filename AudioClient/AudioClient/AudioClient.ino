#include <RS485_non_blocking.h>

 int fAvailable ()
   {
   return Serial.available ();  
   }
 
 int fRead ()
   {
   return Serial.read ();  
   }
 

RS485 myChannel (fRead, fAvailable, NULL, 20);

void setup ()
  {
  Serial.begin (9600);
  myChannel.begin ();
  }  // end of setup

void loop ()
  {
  if (myChannel.update ())
    {
    Serial.write (myChannel.getData (), myChannel.getLength ()); 
    Serial.println ();
    }
  }  // end of loop
