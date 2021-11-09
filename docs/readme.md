# MIDIUSB library

This library allows any microcontroller with native USB capabilities (atmega32u4 based boards or ARM boards) to appear as a MIDI peripheral over USB to a connected computer.

The library is compatible with PluggableUSB-ready cores (AVR core > 1.6.8, SAM core > 1.6.4, SAMD core > 1.6.1)

----

## The library creates the following data type

```
typedef struct {
    uint8_t header;   // cable + command
    uint8_t byte1;    // command + channel
    uint8_t byte2;    // note number or other data
    uint8_t byte3;    // velocity or other data or zero.
} midiEventPacket_t;
```

Note that a USB MIDI message contains an extra byte, compared to a
UART MIDI message.  The `header` byte is added.  Also, while UART MIDI
messages are variable length, a USB MIDI message is always four bytes - any trailing bytes not used by a particular message type will be
zero.

----

## The library exposes the following functions

```
midiEventPacket_t read(void);
```
Read a USB message from USB.
If no message is available, the result "header" will be zero.

```
void sendMIDI(midiEventPacket_t event);
```
Send a single USB message.

```
void flush(void);
```
Wait for all messages to actually be sent.

```
uint32_t available(void);
```
Return the number of USB messages that are available and unread.

```
size_t write(const uint8_t *buffer, size_t size);
```
Write possubly multiple messages to USB.  size_t should be a multiple of four.

----

## Examples

- [MIDIUSB_read](https://github.com/arduino-libraries/MIDIUSB/blob/master/examples/MIDIUSB_read/MIDIUSB_read.ino)
Trivial example that dumps received messages to Serial debug statements.

- [MIDIUSB_write](https://github.com/arduino-libraries/MIDIUSB/blob/master/examples/MIDIUSB_write/MIDIUSB_write.ino)
Trivial example that sends a middle C note lasting 500ms every two second.

- [MIDIUSB_loop](https://github.com/arduino-libraries/MIDIUSB/blob/master/examples/MIDIUSB_loop/MIDIUSB_loop.ino)
Trivial MIDI loopback.  Resends each MIDI message that is received.

- [MIDI Device](https://docs.arduino.cc/tutorials/generic/midi-device)
More complex example: implements a MIDI keyboard that scans 7 buttons and sends MIDI NoteOn/NoteOff messages.

- [MIDIUSB_buzzer](https://github.com/arduino-libraries/MIDIUSB/blob/master/examples/MIDIUSB_buzzer/MIDIUSB_buzzer.ino)
More complex example: implements a MIDI sound device Uses MidiUSB.read() to read MIDI messages; converts noteOn and noteOff messages to actual tones using the TONE library.  Also interprets controlChange to text on Serial, and dumps any other messages in debug format.
                                             
- [MIDIUSB_clock](https://github.com/arduino-libraries/MIDIUSB/blob/master/examples/MIDIUSB_clock/MIDIUSB_clock.ino)
Simple example of beat clock based on MIDI pulse messages received from software.
Interprets message types 0xF8, 0xFA, and 0xFC

- [MIDI Library](https://www.arduino.cc/reference/en/libraries/midi-library/)
MIDIUSB can be used as the "transport layer" for the more advanced Arduino MIDI library.
