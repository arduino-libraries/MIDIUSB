/*
 * MIDIUSB_clock.ino
 * 
 * Simple example of beat clock based on MIDI pulse messages
 * received from software. 
 * 
 * Tested on Leonardo with Ableton.
 * 
 * In preferences go to MIDI Sync. Select device Output
 * and toggle Sync button, change clock type to Pattern.
 * Usually changing Sync Delay is required.
 * 
 * Created: 19/12/2016
 * Author: Ernest Warzocha
 */

#include "MIDIUSB.h"

//Pulse per quarter note. Each beat has 24 pulses.
//Tempo is based on software inner BPM.
int ppqn = 0;

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

void setup() {
  Serial.begin(115200);
}

void loop() {
  
  midiEventPacket_t rx;
  
  do {
    rx = MidiUSB.read();

    //Count pulses and send note 
    if(rx.byte1 == 0xF8){
       ++ppqn;
       
       if(ppqn == 24){
          noteOn(1,48,127);
          MidiUSB.flush();      
          ppqn = 0;
       };
    }
    //Clock start byte
    else if(rx.byte1 == 0xFA){
      noteOn(1,48,127);
      MidiUSB.flush();
      ppqn = 0;
    }
    //Clock stop byte
    else if(rx.byte1 == 0xFC){
      noteOff(1,48,0);
      MidiUSB.flush();
      ppqn = 0;
    };
    
  } while (rx.header != 0);
  
}
