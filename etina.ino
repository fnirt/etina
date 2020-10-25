#include "Adafruit_VL53L0X.h"
#include <Wire.h> // Include the I2C library (required)
#include <SparkFunSX1509.h> // Include SX1509 library
#include <Adafruit_LPS2X.h>
#include <Adafruit_Sensor.h>

#define NOTE_C2  65
#define NOTE_D2  73
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_G2  98
#define NOTE_A2  110
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_D3  147
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_A3  220
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_B5  988
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

int rkPins[]  = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
int rkNotes[] = {NOTE_C4, NOTE_B3, 
                 NOTE_E4, NOTE_D4, 
                 NOTE_G4, NOTE_F4, 
                 NOTE_C5, NOTE_A4, 
                 NOTE_E5, NOTE_B4,
                 NOTE_G4, NOTE_FS4,
                 NOTE_B5, NOTE_A5,
                 NOTE_D5, NOTE_C5,
                 NOTE_G5, NOTE_E5,
                 NOTE_B5, NOTE_FS5};

//int lkPins[]  = {5, 4, 3, 2, 1, 10, 9, 8, 7, 6};
int lkPins[]  = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
int lkNotes[] = {
                NOTE_E5, NOTE_B4,   // 5
                NOTE_C5, NOTE_A4,   // 4
                NOTE_G4, NOTE_F4,   // 3
                NOTE_E4, NOTE_D4,   // 2
                NOTE_C4, NOTE_B3,   // 1
                NOTE_B5, NOTE_FS5,  // 10
                NOTE_G5, NOTE_E5,   // 9
                NOTE_D5, NOTE_C5,   // 8
                NOTE_B5, NOTE_A5,   // 7
                NOTE_G4, NOTE_FS4   // 6
                };

byte numberOfPins = (sizeof(rkPins) / sizeof(rkPins[0]));
byte lnumberOfPins = (sizeof(lkPins) / sizeof(lkPins[0]));

int PowerLight = 12;
int iLED = 13;
int Piezo = 9;

Adafruit_LPS22 lps;

Adafruit_VL53L0X lox = Adafruit_VL53L0X();
int lastreading = 0;
int lastpressurereading = 0;

boolean bPushing = false;

SX1509 io; // Create an SX1509 object
SX1509 lio; // Create an SX1509 object

// SX1509 pin definitions:
// Note: these aren't Arduino pins. They're the SX1509 I/O:
//const int SX1509_LED_PIN = 15; // LED connected to 15 (source ing current)
//const int SX1509_BTN_PIN = 1; // Button connected to 1 (active-low)

bool ledState = false;

void setup()
{
  Serial.begin(9600);
//  while (!Serial) {
//    ; // wait for serial port to connect. Needed for native USB port only
//  }
  Serial.println("Serial Up");
  // Call io.begin(<I2C address>) to initialize the I/O
  // expander. It'll return 1 on success, 0 on fail.
  if (!lio.begin(0x70))
  {
    Serial.println("trying 0x70");
    // If we failed to communicate, turn the pin 13 LED on
    digitalWrite(13, HIGH);
    while (1)
      Serial.println("0x70 failed");
      ; // And loop forever.
  }
    Serial.println("0x70 passed");

  if (!io.begin(0x3E))
  {
    // If we failed to communicate, turn the pin 13 LED on
    Serial.println("trying 0x3E");
    digitalWrite(13, HIGH);
    while (1)
    Serial.println("0x3E failed");
      ; // And loop forever.
  }
    Serial.println("0x3E passed");

  for (byte i = 0; i < numberOfPins; i = i + 1) {
    io.pinMode(rkPins[i], INPUT_PULLUP);
  }

  for (byte i = 0; i < lnumberOfPins; i = i + 1) {
    lio.pinMode(lkPins[i], INPUT_PULLUP);
  }

    Serial.println("Lights and headphones setup");

  pinMode(PowerLight, OUTPUT); 
  pinMode(iLED, OUTPUT); 
  pinMode(Piezo, OUTPUT);
  noTone(Piezo);
    Serial.println("turn power light on");

  digitalWrite(PowerLight, HIGH); // Turn the power LED on
  
    Serial.println("quiet onboard led");
  pinMode(13, OUTPUT); // Use pin 13 LED as debug output
  digitalWrite(13, LOW); // Start it as low

  // Call io.pinMode(<pin>, <mode>) to set any SX1509 pin as
  // either an INPUT, OUTPUT, INPUT_PULLUP, or ANALOG_OUTPUT

//  io.pinMode(SX1509_BTN_PIN, INPUT_PULLUP);

    Serial.println("Adafruit VL53L0X test");
  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }     
    Serial.println("ToF passed");

  Serial.println("Adafruit LPS22 test!");

  // Try to initialize!
  if (!lps.begin_I2C()) {
  //if (!lps.begin_SPI(LPS_CS)) {
  //if (!lps.begin_SPI(LPS_CS, LPS_SCK, LPS_MISO, LPS_MOSI)) {
    Serial.println("Failed to find LPS22 chip");
    while (1) { delay(10); }
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
}


void loop()
{

 VL53L0X_RangingMeasurementData_t measure;

  //Serial.print("Reading a directional measurement... ");
  lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!
  if (measure.RangeStatus != 4) {  // phase failures have incorrect data
    Serial.print("D: "); Serial.print(measure.RangeMilliMeter);
    int thisreading = measure.RangeMilliMeter;
    if (thisreading < 1) {
       Serial.print("      _      ");
      
    } else {
      if (thisreading  < lastreading) {
        Serial.print("        push ");
        bPushing = true;

      } else {
        Serial.print("  pull       ");
        bPushing = false;

      }
      
    }
    lastreading = thisreading;
  } else {
       Serial.print("             ");
  }


  //Serial.print("Reading a pressure measurement... ");
  sensors_event_t temp;
  sensors_event_t pressure;
  lps.getEvent(&pressure, &temp);// get pressure
  if (pressure.pressure != 4) {  // phase failures have incorrect data
    Serial.print("P: "); Serial.print(pressure.pressure);
    int thisreading = pressure.pressure;
    if (thisreading < 1) {
       Serial.print("      _      ");
    } else {
      if (thisreading  > lastpressurereading) {
        Serial.print("        PUSH ");
  //      bPushing = true;
     } else {
        Serial.print("  PULL       ");
    //    bPushing = false;
      }
    }
    lastpressurereading = thisreading;
  } else {
       Serial.print("             ");
  }
  
  // right hand keys
  for (byte i = 0; i < numberOfPins; i = i + 1) {
     checkAndPlay(i);
  }
  // left hand keys
  for (byte i = 0; i < lnumberOfPins; i = i + 1) {
     lcheckAndPlay(i);
  }

  //clean up
//  bPushing = false;
//  delay(10); // Delay a little bit to improve simulation performance
//  noTone(Piezo);
  Serial.println("!");               //Display the read value in the Serial monitor

}

void playsound(double note) {
    Serial.print((String)" note "+note);               //Display the read value in the Serial monitor
    tone(Piezo, note, 500); // plays note
//    delay(1000);
}

void checkAndPlay(int Pin) {
 //   Serial.print((String)" pin  "+Pin);
  int buttonState = io.digitalRead(rkPins[Pin]);
  Serial.print(buttonState);               //Display the read value in the Serial monitor

  // is it lit?
  if (buttonState == 0) {
    double note = 0;
    int place = 0;
    if (bPushing == true) {  // are we pushing?
      place = Pin + Pin;
      note = rkNotes[place];
      playsound(note);
        Serial.print("><");               //Display the read value in the Serial monitor

    } else {  // are we pulling?
      place = Pin + Pin + 1;
      note = rkNotes[place];
        Serial.print("<>");               //Display the read value in the Serial monitor
      playsound(note);
    }    
  } else {
    Serial.print("~");               //Display the read value in the Serial monitor
  }

}

void lcheckAndPlay(int Pin) {
 //   Serial.print((String)" pin  "+Pin);
  int buttonState = lio.digitalRead(lkPins[Pin]);
  Serial.print(buttonState);               //Display the read value in the Serial monitor

  // is it lit?
  if (buttonState == 0) {
    double note = 0;
    int place = 0;
    if (bPushing == true) {  // are we pushing?
      place = Pin + Pin;
      note = lkNotes[place];
      playsound(note);
        Serial.print("><");               //Display the read value in the Serial monitor

    } else {  // are we pulling?
      place = Pin + Pin + 1;
      note = lkNotes[place];
        Serial.print("<>");               //Display the read value in the Serial monitor
      playsound(note);
    }    
  } else {
    Serial.print("~");               //Display the read value in the Serial monitor
  }
}
