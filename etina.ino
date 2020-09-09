
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

int rk1temp = 200;//115;//114
int rk2temp = 300;//190;//186
int rk3temp = 500;//280;//279
int rk4temp = 800;//460;//455
int rk5temp = 1025;//1023

int rk1ptone = NOTE_C4;  
int rk1dtone = NOTE_B3;  
int rk2ptone = NOTE_E4;  
int rk2dtone = NOTE_D4;  
int rk3ptone = NOTE_G4;  
int rk3dtone = NOTE_F4; 
int rk4ptone = NOTE_C5;  
int rk4dtone = NOTE_A4; 
int rk5ptone = NOTE_E5;  
int rk5dtone = NOTE_B4;  

int lk1temp = 200;//115;//114
int lk2temp = 300;//190;//186
int lk3temp = 500;//280;//279
int lk4temp = 800;//460;//455
int lk5temp = 1025;//1023

int lk1ptone = NOTE_C2;  
int lk1dtone = NOTE_G2;  
int lk2ptone = NOTE_G2;  
int lk2dtone = NOTE_B2;  
int lk3ptone = NOTE_C3;  
int lk3dtone = NOTE_D3;  
int lk4ptone = NOTE_E3;  
int lk4dtone = NOTE_F3;  
int lk5ptone = NOTE_G3;  
int lk5dtone = NOTE_A3;  

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
R7  B4  A4
R8  D5  C5
R9  G5  E5
R10 B5  FS5

*/
int iLED = 13;
int iButtonPinL = A1; // keys R1-5
int iButtonPinR = A0; // keys L1-5
int iPushButton = 2;
int Piezo = 12;


boolean bPushing = false;

void setup()
{
  Serial.begin(9600);
  //while(!Serial);
  pinMode(iPushButton, INPUT);
  pinMode(iLED, OUTPUT); 
  pinMode(Piezo, OUTPUT);
  //noTone(Piezo);
}

void loop()
{

  // directional (push) button
  int buttonState = digitalRead(iPushButton);
  if (buttonState == 1) {
    bPushing = true;
    digitalWrite(iLED, HIGH);   // turn the LED on (HIGH is the voltage level)
   } else {
    bPushing = false;
    digitalWrite(iLED, LOW);    // turn the LED off by making the voltage LOW
   }
  Serial.print((String)" pushing: " + bPushing);

  // right hand keys
  int temp = analogRead(iButtonPinR);   //Read the analogue input
  //Serial.println("temp");               //Display the read value in the Serial monitor
  Serial.print((String)" Rtemp: "+temp);               //Display the read value in the Serial monitor
  if (temp < 10 ) 
  {
    //noTone(Piezo);
  }
  else if (temp < rk1temp)                     //Lower limit for first button - if below this limit then no button is pushed and LEDs are turned off
  {
    if (bPushing == 1)  
    { playsound(rk1ptone);   } 
    else 
    { playsound(rk1dtone);   }
  }
  else if (temp < rk2temp)                     //Lower limit for first button - if below this limit then no button is pushed and LEDs are turned off
  {
    if (bPushing == 1)  
    { playsound(rk2ptone);   } 
    else 
    { playsound(rk2dtone);   }
  }
  else if (temp < rk3temp)                     //Lower limit for first button - if below this limit then no button is pushed and LEDs are turned off
  {
    if (bPushing == 1)  
    { playsound(rk3ptone);   } 
    else 
    { playsound(rk3dtone);   }
  }
  else if (temp < rk4temp)                     //Lower limit for first button - if below this limit then no button is pushed and LEDs are turned off
  {
    if (bPushing == 1)  
    { playsound(rk4ptone);   } 
    else 
    { playsound(rk4dtone);   }
  }
  else if (temp < rk5temp)                     //Lower limit for first button - if below this limit then no button is pushed and LEDs are turned off
  {
    if (bPushing == 1)  
    { playsound(rk5ptone);   } 
    else 
    { playsound(rk5dtone);   }
  } else {
      //noTone(Piezo);

  }


  // left hand keys
  temp = analogRead(iButtonPinL);   //Read the analogue input
  //Serial.println("temp");               //Display the read value in the Serial monitor
  Serial.print((String)" Ltemp: " + temp);               //Display the read value in the Serial monitor
  if (temp <10 ) 
  {
    //noTone(Piezo);
  }
  else if (temp < lk1temp)                     //Lower limit for first button - if below this limit then no button is pushed and LEDs are turned off
  {
    if (bPushing == 1)  
    { playsound(lk1ptone);   } 
    else 
    { playsound(lk1dtone);   }
  }
  else if (temp < lk2temp)                     //Lower limit for first button - if below this limit then no button is pushed and LEDs are turned off
  {
    if (bPushing == 1)  
    { playsound(lk2ptone);   } 
    else 
    { playsound(lk2dtone);   }
  }
  else if (temp < lk3temp)                     //Lower limit for first button - if below this limit then no button is pushed and LEDs are turned off
  {
    if (bPushing == 1)  
    { playsound(lk3ptone);   } 
    else 
    { playsound(lk3dtone);   }
  }
  else if (temp < lk4temp)                     //Lower limit for first button - if below this limit then no button is pushed and LEDs are turned off
  {
    if (bPushing == 1)  
    { playsound(lk4ptone);   } 
    else 
    { playsound(lk4dtone);   }
  }
  else if (temp < lk5temp)                     //Lower limit for first button - if below this limit then no button is pushed and LEDs are turned off
  {
    if (bPushing == 1)  
    { playsound(lk5ptone);   } 
    else 
    { playsound(lk5dtone);   }
  } else {
      //noTone(Piezo);

  }

  //clean up
  bPushing = false;
  //delay(10); // Delay a little bit to improve simulation performance
 // noTone(Piezo);
  Serial.println("!");               //Display the read value in the Serial monitor

}

void playsound(double note) {
    Serial.print((String)" note "+note);               //Display the read value in the Serial monitor
    tone(Piezo, note, 500); // plays note
}
 
