# MIDIUSB library

## Functions

### `MidiUSB.read()`

This functions reads data from the USB and packages it into a MIDI packet (midiEventPacket_t struct ). For example, you can use this function to act as a MIDI IN device, getting notes from the PC and transforming them into audible music.

### `MidiUSB.sendMIDI(midiEventPacket_t event)`
Using this function you will send a MIDI encoded packet to the PC. You can use it to produce music by reading inputs and transfer the encoded data to a software synth.

### `MidiUSB.flush()`

This function forces the USB layer to send the data immediately. Since the USB bus is not realtime, a `sendMIDI()` doesn't guarantee the data to be sent with the correct timing unless immediately followed by a `flush()`