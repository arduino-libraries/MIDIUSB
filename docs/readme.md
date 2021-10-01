# MIDIUSB library

These library allows any microcontroller with native USB capabilities (atmega32u4 based boards or ARM boards) to appear as a MIDI peripheral over USB to a connected computer.

The library is compatible with PluggableUSB-ready cores (AVR core > 1.6.8, SAM core > 1.6.4, SAMD core > 1.6.1)

The library exposes the following functions

```
midiEventPacket_t read(void);

void sendMIDI(midiEventPacket_t event);

void flush(void);
```

## Examples

- [MIDI Device](https://docs.arduino.cc/tutorials/generic/midi-device)