
// See http://www.vlsi.fi/fileadmin/datasheets/vs1053.pdf Pg 31
#define VS1053_BANK_DEFAULT 0x00
#define VS1053_BANK_DRUMS1 0x78
#define VS1053_BANK_DRUMS2 0x7F
#define VS1053_BANK_MELODY 0x79

// See http://www.vlsi.fi/fileadmin/datasheets/vs1053.pdf Pg 32 for more!
#define VS1053_GM1_OCARINA 80
#define VS1053_GM1_ACCORDION 22
#define VS1053_GM1_HARMONICA 23

#define MIDI_NOTE_ON  0x90
#define MIDI_NOTE_OFF 0x80
#define MIDI_CHAN_MSG 0xB0
#define MIDI_CHAN_BANK 0x00
#define MIDI_CHAN_VOLUME 0x07
#define MIDI_CHAN_PROGRAM 0xC0
#define MIDI_CHAN_ALLOFFA 0x7B
#define MIDI_CHAN_ALLOFFB 0x7C
#define MIDI_CHAN_ALLOFFC 0x7D
#define MIDI_CMD_HOLD 0x40
#define MIDI_CMD_SUS 0x42
#define MIDI_CMD_SILENCE 0x78

#if defined(ESP8266) || defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__)
  #define VS1053_MIDI Serial
#else
  // anything else? use the hardware serial1 port
  #define VS1053_MIDI Serial1
#endif


void midiSetInstrument(uint8_t chan, uint8_t inst) {
  if (chan > 15) return;
  inst --; // page 32 has instruments starting with 1 not 0 :(
  if (inst > 127) return;
  
  VS1053_MIDI.write(MIDI_CHAN_PROGRAM | chan);  
  delay(10);
  VS1053_MIDI.write(inst);
  delay(10);
}


void midiSetChannelVolume(uint8_t chan, uint8_t vol) {
  if (chan > 15) return;
  if (vol > 127) return;
  
  VS1053_MIDI.write(MIDI_CHAN_MSG | chan);
  VS1053_MIDI.write(MIDI_CHAN_VOLUME);
  VS1053_MIDI.write(vol);
}

void midiSetChannelBank(uint8_t chan, uint8_t bank) {
  if (chan > 15) return;
  if (bank > 127) return;
  
  VS1053_MIDI.write(MIDI_CHAN_MSG | chan);
  VS1053_MIDI.write((uint8_t)MIDI_CHAN_BANK);
  VS1053_MIDI.write(bank);
}

void midiNoteOn(uint8_t chan, uint8_t n, uint8_t vel) {
  if (chan > 15) return;
  if (n > 127) return;
  if (vel > 127) return;
//    Serial.print(n);
 //   Serial.println(" on");
  
  VS1053_MIDI.write(MIDI_NOTE_ON | chan);
  VS1053_MIDI.write(n);
  VS1053_MIDI.write(vel);
}

void midiNoteOff(uint8_t chan, uint8_t n, uint8_t vel) {
  if (chan > 15) return;
  if (n > 127) return;
  if (vel > 127) return;
//    Serial.print(n);
//    Serial.println(" off");

  VS1053_MIDI.write(MIDI_NOTE_OFF | chan);
  VS1053_MIDI.write(n);
  VS1053_MIDI.write(vel);
}
void midiSilence(uint8_t chan, uint8_t n, uint8_t vel) {
  if (chan > 15) return;
  if (n > 127) return;
  if (vel > 127) return;
  
  VS1053_MIDI.write(MIDI_CHAN_ALLOFFA | chan);
  VS1053_MIDI.write(n);
  VS1053_MIDI.write(vel);
  delay(10);

  VS1053_MIDI.write(MIDI_CHAN_ALLOFFB | chan);
  VS1053_MIDI.write(n);
  VS1053_MIDI.write(vel);
  delay(10);

  VS1053_MIDI.write(MIDI_CHAN_ALLOFFC | chan);
  VS1053_MIDI.write(n);
  VS1053_MIDI.write(vel);
  delay(10);
}
void midiSus(uint8_t chan, uint8_t n, uint8_t vel) {
  
  VS1053_MIDI.write(MIDI_CHAN_MSG| chan);
  VS1053_MIDI.write(MIDI_CMD_SUS );
  VS1053_MIDI.write(127);
  delay(10);

}
void midiSilent(uint8_t chan, uint8_t n, uint8_t vel) {
  
  VS1053_MIDI.write(MIDI_CHAN_MSG| chan);
  VS1053_MIDI.write(MIDI_CMD_SILENCE );
  VS1053_MIDI.write(127);
  delay(10);

}
