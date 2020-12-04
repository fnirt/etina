#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_VL53L0X.h"
#include "notes.h"
#include "instruments.h"
#include "midihelper.h"
#include <Wire.h> // Include the I2C library (required)
#include <SparkFunSX1509.h> // Include SX1509 library
#include <Adafruit_LPS2X.h>
#include <Adafruit_Sensor.h>
#include "movingAvg.h"                  // https://github.com/JChristensen/movingAvg
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)

/*

  Todo:

  implement potentiometer as volume adjustment


  hardware
    fix headphone connection
    power display

  Done:
    white end cap for left hand
    implement LED display
    power
    note played
    remove old LED
    display on Serial before display
    add instruments include
    implement multi-button press to change instrument
    add potentiometer to Analog pin
    label button connectors
    fix all buttons
    reimplement power switch
    add flex sensor
    reimplement power LED
    remove hardcoded direction
    implement multi-sensor pushpull averaging
    solder flex sensor to main board
    solder main board flex channels

*/



/*

    p  d
  L1  C2  G2
  L2  G2  B2
  L3  C3  D3
  L4  E3  F3
  L5  G3  A3

  L6  B2  A2
  L7  D3  FS3
  L8  G3  A3
  L9  B3  C4
  L10 D4  E4

  R1  C4  B3
  R2  E4  D4
  R3  G4  F4
  R4  C5  A4
  R5  E5  B4

  R6  G4  FS4
  R7  B5  A5
  R8  D5  C5
  R9  G5  E5
  R10 B5  FS5

*/

/****************************************************************


   Initialize information about buttons, pins, and notes


 ****************************************************************/

int pins[]  = {5, 4, 3, 2, 1, 10, 9, 8, 7, 6,
               1, 2, 3, 4, 5, 6, 7, 8, 9, 10
              };

char names[] = {'C', 'G', 'G', 'B', 'C', 'D', 'E', 'F', 'G', 'A',
                'B', 'A', 'D', 'f', 'G', 'A', 'B', 'C', 'D', 'E',
                'C', 'B', 'E', 'D', 'G', 'F', 'C', 'A', 'E', 'B',
                'G', 'f', 'B', 'A', 'D', 'C', 'G', 'E', 'B', 'f'
               };

int notes[] = {NOTE_C2,  NOTE_G2,
               NOTE_G2,  NOTE_B2,
               NOTE_C3,  NOTE_D3,
               NOTE_E3,  NOTE_F3,
               NOTE_G3,  NOTE_A3,
               NOTE_B2,  NOTE_A2,
               NOTE_D3,  NOTE_FS3,
               NOTE_G3,  NOTE_A3,
               NOTE_B3,  NOTE_C4,
               NOTE_D4,  NOTE_E4,
               NOTE_C4,  NOTE_B3,
               NOTE_E4,  NOTE_D4,
               NOTE_G4,  NOTE_F4,
               NOTE_C5,  NOTE_A4,
               NOTE_E5,  NOTE_B4,
               NOTE_G4,  NOTE_FS4,
               NOTE_B5,  NOTE_A5,
               NOTE_D5,  NOTE_C5,
               NOTE_G5,  NOTE_E5,
               NOTE_B5,  NOTE_FS5
              };

int laststatus[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//char *instrumentname[] = {"1 Acoustic Grand Pian",
//                          "22 Accordion         ",
//                          "23 Harmonica         ",
//                          "28 Electric Guitar (c",
//                          "57 Trumpet           ",
//                          "79 Whistle           ",
//                          "80 Ocarina           ",
//                          "110 Bag Pipe         ",
//                          "119 Synth Drum       ",
//                          "128 Gunshot          "
//                         };
//int instrumentnumber[] = {1,
//                          22,
//                          23,
//                          28,
//                          57,
//                          79,
//                          80,
//                          110,
//                          119,
//                          128
//                         };

byte numberOfPins = (sizeof(pins) / sizeof(pins[0]));
byte numberOfInstruments = (sizeof(instrumentnumber) / sizeof(instrumentnumber[0])) - 1;
int choseninstrument = 2;

movingAvg avgFlex(10);                  // define the moving average object

int PowerLight = 12;
int iLED = 13;
bool ledState = false;
int volume = 255;
int thisInstrument = choseninstrument; //VS1053_GM1_HARMONICA;

/****************************************************************


   Initialize sensors


 ****************************************************************/

SX1509 io;                                    // Create a Right SX1509 object
SX1509 lio;                                   // Create a Left SX1509 object
Adafruit_SSD1306  display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int lastflexreading = 0;
int flexinputPin = A2;

boolean bPushing = false;
int pushpullaggregator = 0;

/****************************************************************


   Setup


 ****************************************************************/

void setup()
{
  Serial.begin(9600);
  //  while (!Serial) {
  //    ; // wait for serial port to connect. Needed for native USB port only
  //  }

  /*
     Check Left IO expander
  */

  Serial.println("Checking IO expanders");
  Serial.println("trying 0x70 left expander");
  if (!lio.begin(0x70)) { //1 on success, 0 on fail.
    digitalWrite(13, HIGH);
    while (1)
      Serial.println("0x70 failed");      ; // And loop forever.
  }
  Serial.println("io exp R 0x70 passed");

  /*
     Check right IO expander
  */
  Serial.println("trying 0x3E right expander");
  if (!io.begin(0x3E))  {
    digitalWrite(13, HIGH);
    while (1)
      Serial.println("0x3E failed");      ; // And loop forever.
  }
  Serial.println("io exp L 0x3E passed");

  // light up all the pins
  for (byte i = 0; i < numberOfPins; i = i + 1) {
    if (i < 10) {
      lio.pinMode(pins[i], INPUT_PULLUP);
    } else {
      io.pinMode(pins[i], INPUT_PULLUP);
    }
  }

  /*
     Lights
  */
  Serial.println("Lights setup");
  pinMode(iLED, OUTPUT); // Use pin 13 LED as debug output

  Serial.println("turn power light on");
  pinMode(PowerLight, OUTPUT);
  digitalWrite(PowerLight, HIGH); // Turn the power LED on

  Serial.println("quiet onboard led");
  digitalWrite(iLED, LOW); // Start it as low

  /*
     Music Maker
  */
  Serial.println("VS1053 MIDI test");
  VS1053_MIDI.begin(31250); // MIDI uses a 'strange baud rate'
  midiSetChannelBank(0, VS1053_BANK_MELODY);
  midiSetChannelVolume(0, volume);
  midiSetInstrument(0, thisInstrument);
      midiSetInstrument(0, instrumentnumber[choseninstrument]);

  avgFlex.begin();

  /*
     Display
  */
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.clearDisplay();


      midiNoteOn(0, NOTE_C4, 127); // channel, note, velocity
      delay(50);
      midiNoteOn(0, NOTE_C4, 127); // channel, note, velocity
      delay(50);
      midiNoteOn(0, NOTE_C4, 127); // channel, note, velocity
      delay(50);
      midiNoteOn(0, NOTE_C4, 127); // channel, note, velocity
      delay(50);
      midiNoteOn(0, NOTE_C4, 127); // channel, note, velocity
      delay(50);
      midiNoteOn(0, NOTE_C4, 127); // channel, note, velocity
      delay(50);
      midiNoteOn(0, NOTE_C4, 127); // channel, note, velocity
      delay(50);
      midiNoteOn(0, NOTE_C4, 127); // channel, note, velocity
      delay(50);
      midiNoteOn(0, NOTE_C4, 127); // channel, note, velocity
      delay(50);
      midiNoteOn(0, NOTE_C4, 127); // channel, note, velocity
      delay(50);
      midiNoteOff(0, NOTE_C4, 127); // channel, note, velocity
      delay(50);

      delay(3000);
      midiNoteOn(0, NOTE_C4, 127); // channel, note, velocity
      delay(3000);
      midiNoteOff(0, NOTE_C4, 127); // channel, note, velocity

      delay(3000);
      midiNoteOn(0, NOTE_C4, 127); // channel, note, velocity
      delay(50);
      midiNoteOff(0, NOTE_C4, 127); // channel, note, velocity
      midiNoteOn(0, NOTE_C4, 127); // channel, note, velocity
      delay(50);
      midiNoteOff(0, NOTE_C4, 127); // channel, note, velocity
      midiNoteOn(0, NOTE_C4, 127); // channel, note, velocity
      delay(50);
      midiNoteOff(0, NOTE_C4, 127); // channel, note, velocity
      midiNoteOn(0, NOTE_C4, 127); // channel, note, velocity
      delay(50);
      midiNoteOff(0, NOTE_C4, 127); // channel, note, velocity
      midiNoteOn(0, NOTE_C4, 127); // channel, note, velocity
      delay(50);
      midiNoteOff(0, NOTE_C4, 127); // channel, note, velocity
      midiNoteOn(0, NOTE_C4, 127); // channel, note, velocity
      delay(50);
      midiNoteOff(0, NOTE_C4, 127); // channel, note, velocity
      midiNoteOn(0, NOTE_C4, 127); // channel, note, velocity
      delay(50);
      midiNoteOff(0, NOTE_C4, 127); // channel, note, velocity
      midiNoteOn(0, NOTE_C4, 127); // channel, note, velocity
      delay(50);
      midiNoteOff(0, NOTE_C4, 127); // channel, note, velocity
      midiNoteOn(0, NOTE_C4, 127); // channel, note, velocity
      delay(50);
      midiNoteOff(0, NOTE_C4, 127); // channel, note, velocity
      midiNoteOn(0, NOTE_C4, 127); // channel, note, velocity
      delay(50);
      midiNoteOff(0, NOTE_C4, 127); // channel, note, velocity
      delay(50);

  midiSus(0, 0, 127);
  
}




/****************************************************************


   Main loop


 ****************************************************************/

void loop()
{

  int status[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  char pushstring[] = {"                     "};
  char statstring[] = {"                     "};
  char notestring[] = {"                     "};
  char  keystring[] = {"....!....! ....!....!"};

  pushpullaggregator = 0;

  /*   Flex read  */
  int flexreading = avgFlex.reading(analogRead(flexinputPin));

  /*
     Set push/pull
  */

  if (flexreading < 1) {
    Serial.print("      _      ");
  } else {
    if (flexreading  < lastflexreading) {
      strncpy(pushstring, "   PUSH", 20);
      bPushing = true;
    } else {
      strncpy(pushstring, "              PULL   ", 20);
      bPushing = false;
    }
  }
  lastflexreading = flexreading;

  /*
     Look for console commands
    String command;

    if(Serial.available()){
      command = Serial.readStringUntil('\n');
      command = command.substring(0,1);

      Serial.println("?");
      Serial.println(command);
      Serial.println("?");

      if(command.equals("a")) {
        Serial.println("add");
      }
      Serial.println("");
      delay(1000);
    }
  */

  /****************************************************************


    Play the music


  ****************************************************************/


  int thisvalue = 0;
  boolean silent = true;
  for (int i = 0; i < numberOfPins; i = i + 1) {
    if (i < (numberOfPins / 2)) {
      thisvalue = lio.digitalRead(pins[i]);
    } else {
      thisvalue = io.digitalRead(pins[i]);
    }

    if (thisvalue == 0) {
      silent  = false;
      int place = i + i;
      int offset = 49;
      int spacer = 0;

      if (bPushing == 1) {
      } else  {
        place += 1;
      }
      int midivalue = notes[place];
      status[i] = midivalue;
      int displaynote = names[place];

      if (i == 9) {
        offset = 39;
      }
      if (i == 19) {
        offset = 39;
      }

      if (i >= 10) {
        spacer += 1;
        offset -= 10;
      }
      keystring[i + spacer] = i + offset;
      notestring[i + spacer] = displaynote;
    } else {
      midiNoteOff(0, notes[i], 127); // channel, note, velocity
    }
  }
  if (silent == true ) {
    shhh();
  }
  for (int i = 0; i < numberOfPins; i = i + 1) {
    Serial.print(status[i]);

    if (status[i] > 0) { // if pushing
  //    Serial.print("1");      
      if (status[i] != laststatus[i]) {  // if the notes don't match
    //    Serial.print("2");      
        midiNoteOff(0, laststatus[i], 127); // channel, note, velocity  // off with the old 
        midiNoteOn(0, status[i], 127); // channel, note, velocity      // on with the new
      }
    } else {  // not pushing
   //   Serial.print("3");      
      if (laststatus[i] > 0) {  // was pushing?
    //    Serial.print("4");      
        midiNoteOff(0, laststatus[i], 127); // channel, note, velocity //  turn off then
        //  also the secondary note?   midiNoteOff(0, status[i], 127); // channel, note, velocity
      }
    }
    laststatus[i] = status[i];   // memorize note
    status[i] = 0;  // forget note
  }
   // delay(1000);

  Serial.print("CI:");               //Display the read value in the Serial monitor
  Serial.print(choseninstrument);               //Display the read value in the Serial monitor
  Serial.print(" of ");               //Display the read value in the Serial monitor
  Serial.println("numberOfInstruments");               //Display the read value in the Serial monitor

  if (laststatus[0] > 0) {
    if (laststatus[numberOfPins - 1] > 0) {
      choseninstrument += 1;
      if (choseninstrument > numberOfInstruments) {
        choseninstrument = 0;
      }
     // shhh();
     // delay(20);
      midiSetInstrument(0, instrumentnumber[choseninstrument]);
        midiSus(0, 0, 127);

    }
  }
  if (laststatus[1] > 0) {
    if (laststatus[numberOfPins - 2] > 0) {
      choseninstrument -= 1;
      if (choseninstrument < 0) {
        choseninstrument = numberOfInstruments;
      }
     // shhh();
     // delay(20);
      midiSetInstrument(0, instrumentnumber[choseninstrument]);
        midiSus(0, 0, 127);

    }
  }
  Serial.print("CI:");               //Display the read value in the Serial monitor
  Serial.print(choseninstrument);               //Display the read value in the Serial monitor
  Serial.print(" of ");               //Display the read value in the Serial monitor
  Serial.println(numberOfInstruments);               //Display the read value in the Serial monitor
  Serial.print(" which is ");               //Display the read value in the Serial monitor
  Serial.println(instrumentname[choseninstrument]);               //Display the read value in the Serial monitor

  strcpy(statstring, instrumentname[choseninstrument]);
  display.setCursor(0, 0);     // Start at top-left corner
  display.clearDisplay();

  display.println(statstring);
  display.println(pushstring);
  display.println(notestring);
  display.println(keystring);

  //Serial.println(statstring);
  //Serial.println(pushstring);
  //Serial.println(notestring);
  //Serial.println(keystring);
  //Serial.println("______________________");
  Serial.println("!");               //Display the read value in the Serial monitor

  display.display();
  delay(20);

}

void shhh() {
    midiSilence(0, 0, 127); // channel, note, velocity      
    Serial.print("Shh");      

//  for (int i = 0; i < (numberOfPins * 2); i = i + 1) {
//    midiNoteOff(0, notes[i], 127); // channel, note, velocity
//  }
}
