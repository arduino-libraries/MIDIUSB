/*
 * MIDIUSB_test.ino
 *
 * Created: 4/6/2015 10:47:08 AM
 * Author: gurbrinder grewal
 * Modified by Arduino LLC (2015)
 */ 

#include "MIDIUSB.h"

// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

void setup() {
  Serial1.begin(115200);
  MidiUSB.attachInterrupt(onEvent);
}

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

void loop() {

}

void onEvent(int ep) {
    USB->DEVICE.INTENCLR.reg = 0xFF; //SAMD interrupts continuously without this.
    midiEventPacket_t rx;
    do {
      rx = MidiUSB.read();
      if (rx.header != 0) {
        Serial1.print("Received: ");
        Serial1.print(rx.header, HEX);
        Serial1.print("-");
        Serial1.print(rx.byte1, HEX);
        Serial1.print("-");
        Serial1.print(rx.byte2, HEX);
        Serial1.print("-");
        Serial1.println(rx.byte3, HEX);
      }
    } while (rx.header != 0);
}
