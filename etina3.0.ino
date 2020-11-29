#include "Adafruit_VL53L0X.h"
#include "notes.h"
#include "midihelper.h"
#include <Wire.h> // Include the I2C library (required)
#include <SparkFunSX1509.h> // Include SX1509 library
#include <Adafruit_LPS2X.h>
#include <Adafruit_Sensor.h>
#include "movingAvg.h"                  // https://github.com/JChristensen/movingAvg

/*

  Todo:

  add instruments include

  remove hardcoded direction

  implement potentiometer as volume adjustment
  reimplement power LED

  implement multi-button press to change instrument
  display on Serial before display

  implement LED display
  power
  note played
  remove old LED

  hardware
  add flex sensor
  add potentiometer to Analog pin
  fix headphone connection
  label button connectors
  fix all buttons
  reimplement power switch

  Done:
  implement multi-sensor pushpull averaging

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

int rkPins[]  = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
int rkNotes[] = {
  NOTE_C4, NOTE_B3,
  NOTE_E4, NOTE_D4,
  NOTE_G4, NOTE_F4,
  NOTE_C5, NOTE_A4,
  NOTE_E5, NOTE_B4,
  NOTE_G4, NOTE_FS4,
  NOTE_B5, NOTE_A5,
  NOTE_D5, NOTE_C5,
  NOTE_G5, NOTE_E5,
  NOTE_B5, NOTE_FS5
};

int lkPins[]  = {5, 4, 3, 2, 1, 10, 9, 8, 7, 6};
int lkNotes[] = {
  NOTE_C2, NOTE_G2,  // 1
  NOTE_G2, NOTE_B2,  // 2
  NOTE_C3, NOTE_D3,  // 3
  NOTE_E3, NOTE_F3,  // 4
  NOTE_G3, NOTE_A3,  // 5
  NOTE_B2, NOTE_A2,  // 6
  NOTE_D3, NOTE_FS3, // 7
  NOTE_G3, NOTE_A3,  // 8
  NOTE_B3, NOTE_C4,  // 9
  NOTE_D4, NOTE_E4   // 10
};

byte numberOfPins = (sizeof(rkPins) / sizeof(rkPins[0]));
byte lnumberOfPins = (sizeof(lkPins) / sizeof(lkPins[0]));

movingAvg avgToF(10);                  // define the moving average object
movingAvg avgPressure(10);                  // define the moving average object
movingAvg avgFlex(10);                  // define the moving average object

int PowerLight = 12;
int iLED = 13;
bool ledState = false;
int volume = 190;
int thisInstrument = VS1053_GM1_HARMONICA;

/****************************************************************


   Initialize sensors


 ****************************************************************/

Adafruit_LPS22 lps;                           // pressure sensor
Adafruit_VL53L0X lox = Adafruit_VL53L0X();    // TofF
SX1509 io;                                    // Create a Right SX1509 object
SX1509 lio;                                   // Create a Left SX1509 object

int lastreading = 0;
int lastpressurereading = 0;
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
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

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
    io.pinMode(rkPins[i], INPUT_PULLUP);
    lio.pinMode(lkPins[i], INPUT_PULLUP);
  }

  /*
     Lights
  */
  Serial.println("Lights setup");
  pinMode(iLED, OUTPUT); // Use pin 13 LED as debug output

  //  Serial.println("turn power light on");
  //  pinMode(PowerLight, OUTPUT);
  //  digitalWrite(PowerLight, HIGH); // Turn the power LED on

  Serial.println("quiet onboard led");
  digitalWrite(iLED, LOW); // Start it as low

  /*
     ToF sensor
  */

  Serial.println("Adafruit VL53L0X test");
  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while (1);
  }
  Serial.println("ToF passed");

  /*
     Pressure sensor
  */
  Serial.println("Adafruit LPS22 test");
  if (!lps.begin_I2C()) {
    Serial.println("Failed to find LPS22 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("LPS22 Found!");

  lps.setDataRate(LPS22_RATE_10_HZ);
  Serial.print("Data rate set to: ");
  switch (lps.getDataRate()) {
    case LPS22_RATE_ONE_SHOT: Serial.println("One Shot / Power Down"); break;
    case LPS22_RATE_1_HZ: Serial.println("1 Hz"); break;
    case LPS22_RATE_10_HZ: Serial.println("10 Hz"); break;
    case LPS22_RATE_25_HZ: Serial.println("25 Hz"); break;
    case LPS22_RATE_50_HZ: Serial.println("50 Hz"); break;
  }

  /*
     Music Maker
  */
  Serial.println("VS1053 MIDI test");
  VS1053_MIDI.begin(31250); // MIDI uses a 'strange baud rate'
  midiSetChannelBank(0, VS1053_BANK_MELODY);
  midiSetChannelVolume(0, volume);
  midiSetInstrument(0, thisInstrument);

  avgToF.begin();
  avgPressure.begin();
  avgFlex.begin();
}

/****************************************************************


   Main loop


 ****************************************************************/

void loop()
{

  /*     ToF read  */
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!
  int TOFreading = avgToF.reading(measure.RangeStatus);
  Serial.print("  D: ");
  ActOnReading(TOFreading, lastreading);
  lastreading = TOFreading;

  /*   Pressure read  */
  sensors_event_t temp;
  sensors_event_t pressure;
  lps.getEvent(&pressure, &temp);// get pressure
  int thispressurereading = avgPressure.reading(pressure.pressure);
  Serial.print("  P: ");
  ActOnReading(thispressurereading, lastpressurereading);
  lastpressurereading = thispressurereading;

  /*   Flex read  */
  int flexreading = avgFlex.reading(analogRead(flexinputPin));
  Serial.print("  F: ");
  ActOnReading(flexreading, lastflexreading);
  lastflexreading = flexreading;

  /*
     Set push/pull based on aggregator
  */

  if (pushpullaggregator > 0) {
    bPushing = true;
  }
  if (pushpullaggregator < 0) {
    bPushing = false;
  }

  // hardcode pushing for testing
  bPushing = true;
  String command;

  /*
     Look for console commands

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

  for (byte i = 0; i < numberOfPins; i = i + 1) {
    int place = i + i;
    if (bPushing == false) { // pull offset
      place += 1;
    }
    ButtonCheck (lkNotes[place], io.digitalRead(lkPins[i]));
    ButtonCheck (rkNotes[place], lio.digitalRead(rkPins[i]));
  }
  Serial.println("!");               //Display the read value in the Serial monitor
}

void ActOnReading(int thisreading, int lastreading) {
  Serial.print(thisreading);
  if (thisreading < 1) {
    Serial.print("      _      ");
  } else {
    if (thisreading  > lastreading) {
      Serial.print("        PUSH ");
      pushpullaggregator -= 1;
    } else {
      Serial.print("  PULL       ");
      pushpullaggregator += 1;
    }
  }
}

void ButtonCheck(int thisNote, int buttonState) {
  if (buttonState == 0) {
    Serial.print("  ON:  ");
    Serial.print(thisNote);
    midiNoteOn(0, thisNote, 127); // channel, note, velocity
  } else {
    midiNoteOff(0, thisNote, 127); // channel, note, velocity
  }
}
