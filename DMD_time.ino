/*--------------------------------------------------------------------------------------
  MODIFIED FROM ORIGINAL EXAMPLE
--------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------

  dmd_clock_readout.cpp
   Example clock readout project for the Freetronics DMD, a 512 LED matrix display
   panel arranged in a 32 x 16 layout.

  Copyright (C) 2011 Marc Alexander (info <at> freetronics <dot> com)

  See http://www.freetronics.com/dmd for resources and a getting started guide.

  Note that the DMD library uses the SPI port for the fastest, low overhead writing to the
  display. Keep an eye on conflicts if there are any other devices running from the same
  SPI port, and that the chip select on those devices is correctly set to be inactive
  when the DMD is being written to.

  USAGE NOTES
  -----------

  - Place the DMD library folder into the "arduino/libraries/" folder of your Arduino installation.
  - Get the TimerOne library from here: http://code.google.com/p/arduino-timerone/downloads/list
   or download the local copy from the DMD library page (which may be older but was used for this creation)
   and place the TimerOne library folder into the "arduino/libraries/" folder of your Arduino installation.
  - Restart the IDE.
  - In the Arduino IDE, you can open File > Examples > DMD > dmd_demo, or dmd_clock_readout, and get it
   running straight away!

   The DMD comes with a pre-made data cable and DMDCON connector board so you can plug-and-play straight
   into any regular size Arduino Board (Uno, Freetronics Eleven, EtherTen, USBDroid, etc)

   Please note that the Mega boards have SPI on different pins, so this library does not currently support
   the DMDCON connector board for direct connection to Mega's, please jumper the DMDCON pins to the
   matching SPI pins on the other header on the Mega boards.

  This example code is in the public domain.
  The DMD library is open source (GPL), for more see DMD.cpp and DMD.h

  --------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------
  Includes
  --------------------------------------------------------------------------------------*/
#include <SPI.h>        //SPI.h must be included as DMD is written by SPI (the IDE complains otherwise)
#include <DMD.h>        //
#include <TimerOne.h>   //
#include "Arial_black_16.h"
#include "pitches.h"

const int buttonPin = 3;     // the number of the pushbutton pin
int buttonState = 0;         // variable for reading the pushbutton status

// notes in the melody:
int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

//Fire up the DMD library as dmd
DMD dmd(1, 1);

/*--------------------------------------------------------------------------------------
  Interrupt handler for Timer1 (TimerOne) driven DMD refresh scanning, this gets
  called at the period set in Timer1.initialize();
  --------------------------------------------------------------------------------------*/
void ScanDMD()
{
  dmd.scanDisplayBySPI();
}

/*--------------------------------------------------------------------------------------
  Show clock numerals on the screen from a 4 digit time value, and select whether the
  flashing colon is on or off
  --------------------------------------------------------------------------------------*/
void ShowClockNumbers( unsigned int uiTime, byte bColonOn )
{
  dmd.clearScreen(true);
  dmd.drawChar(  1,  1, '0' + ((uiTime % 1000) / 100),  GRAPHICS_NORMAL ); // hundreds
  dmd.drawChar( 14,  1, '0' + ((uiTime % 100)  / 10),   GRAPHICS_NORMAL ); // tens
  dmd.drawChar( 23,  1, '0' + (uiTime % 10),          GRAPHICS_NORMAL ); // units
  if ( bColonOn )
    dmd.drawChar( 10,  1, ':', GRAPHICS_OR     );   // clock colon overlay on
  else
    dmd.drawChar( 10,  1, ':', GRAPHICS_NOR    );   // clock colon overlay off
}

// Function to concatenate two integers into one
int concat(int minutes, int seconds)
{
  // Convert both the integers to string
  String s1 = String(minutes);
  String s2;  

  if (0 <= seconds && seconds < 10) // between 0 and 10, so add a leading "0" for the tens column.
  {
    s2 = "0" + String(seconds);
  }
  else
  {
    s2 = String(seconds);
  }  

  // Concatenate both strings
  String s = s1 + s2;

  // Convert the concatenated string
  // to integer
  int c = s.toInt();

  // return the formed integer
  return c;
}

void playMelody() {
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 8; thisNote++) {

    // to calculate the note duration, take one second
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(2, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(2);
  }
}

void flashNumbers(int number){
  for (int i = 0; i <= 3; i++) {
    dmd.clearScreen(true);
    delay(500);
    ShowClockNumbers(number, true );
    delay(500);
  }
}

/*--------------------------------------------------------------------------------------
  setup
  Called by the Arduino architecture before the main loop begins
  --------------------------------------------------------------------------------------*/
void setup(void)
{
  Serial.begin(9600);

  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);

  //initialize TimerOne's interrupt/CPU usage used to scan and refresh the display
  Timer1.initialize( 5000 );           //period in microseconds to call ScanDMD. Anything longer than 5000 (5ms) and you can see flicker.
  Timer1.attachInterrupt( ScanDMD );   //attach the Timer1 interrupt to ScanDMD which goes to dmd.scanDisplayBySPI()

  //clear/init the DMD pixels held in RAM
  dmd.clearScreen( true );   //true is normal (all pixels off), false is negative (all pixels on)
  dmd.selectFont(Arial_Black_16);

}

/*--------------------------------------------------------------------------------------
  loop
  Arduino architecture main loop
  --------------------------------------------------------------------------------------*/
void loop(void)
{
  unsigned int timeToPresent = 0; //in minutes, no more than "9"

  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);
  
  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH:
  if (buttonState == HIGH) {
  // turn timer on:
      for (int minutes = timeToPresent; minutes >= 0; minutes--) {
        for (int seconds = 5; seconds >= 0; seconds--) {
          
          int timeLeft = concat(minutes, seconds);
                
          ShowClockNumbers(timeLeft, true );
          delay(1000);
          
          //When time's up, make it obvious!
          if(timeLeft == 000){
            playMelody();
            flashNumbers(timeLeft);
          }
        }
      }      
  } else {
  // turn timer off:
    dmd.clearScreen(true);
  }  
}

