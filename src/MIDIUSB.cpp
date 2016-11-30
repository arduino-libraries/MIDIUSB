/*
** Copyright (c) 2015, Gary Grewal
** Permission to use, copy, modify, and/or distribute this software for
** any purpose with or without fee is hereby granted, provided that the
** above copyright notice and this permission notice appear in all copies.
**
** THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
** WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
** BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
** OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
** WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
** ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
** SOFTWARE.
*/

#include "MIDIUSB.h"

#define MIDI_AC_INTERFACE 	pluggedInterface	// MIDI AC Interface
#define MIDI_INTERFACE 		((uint8_t)(pluggedInterface+1))
#define MIDI_FIRST_ENDPOINT pluggedEndpoint
#define MIDI_ENDPOINT_OUT	pluggedEndpoint
#define MIDI_ENDPOINT_IN	((uint8_t)(pluggedEndpoint+1))

#define MIDI_RX MIDI_ENDPOINT_OUT
#define MIDI_TX MIDI_ENDPOINT_IN

struct ring_bufferMIDI
{
	midiEventPacket_t midiEvent[MIDI_BUFFER_SIZE];
	volatile uint32_t head;
	volatile uint32_t tail;
};

ring_bufferMIDI midi_rx_buffer = {{0,0,0,0 }, 0, 0};

MIDI_ MidiUSB;

int MIDI_::getInterface(uint8_t* interfaceNum)
{
	interfaceNum[0] += 2;	// uses 2 interfaces
	MIDIDescriptor _midiInterface =
	{
		D_IAD(MIDI_AC_INTERFACE, 2, MIDI_AUDIO, MIDI_AUDIO_CONTROL, 0),
		D_INTERFACE(MIDI_AC_INTERFACE,0,MIDI_AUDIO,MIDI_AUDIO_CONTROL,0),
		D_AC_INTERFACE(0x1, MIDI_INTERFACE),
		D_INTERFACE(MIDI_INTERFACE,2, MIDI_AUDIO,MIDI_STREAMING,0),
		D_AS_INTERFACE,
		D_MIDI_INJACK(MIDI_JACK_EMD, 0x1),
		D_MIDI_INJACK(MIDI_JACK_EXT, 0x2),
		D_MIDI_OUTJACK(MIDI_JACK_EMD, 0x3, 1, 2, 1),
		D_MIDI_OUTJACK(MIDI_JACK_EXT, 0x4, 1, 1, 1),
		D_MIDI_JACK_EP(USB_ENDPOINT_OUT(MIDI_ENDPOINT_OUT),USB_ENDPOINT_TYPE_BULK,MIDI_BUFFER_SIZE),
		D_MIDI_AC_JACK_EP(1, 1),
		D_MIDI_JACK_EP(USB_ENDPOINT_IN(MIDI_ENDPOINT_IN),USB_ENDPOINT_TYPE_BULK,MIDI_BUFFER_SIZE),
		D_MIDI_AC_JACK_EP (1, 3)
	};
	return USB_SendControl(0, &_midiInterface, sizeof(_midiInterface));
}

bool MIDI_::setup(USBSetup& setup __attribute__((unused)))
{
	//Support requests here if needed. Typically these are optional
	return false;
}

int MIDI_::getDescriptor(USBSetup& setup __attribute__((unused)))
{
	return 0;
}

uint8_t MIDI_::getShortName(char* name)
{
	memcpy(name, "MIDI", 4);
	return 4;
}

void MIDI_::accept(void)
{
	ring_bufferMIDI *buffer = &midi_rx_buffer;
	uint32_t i = (uint32_t)(buffer->head+1) % MIDI_BUFFER_SIZE;

	// if we should be storing the received character into the location
	// just before the tail (meaning that the head would advance to the
	// current location of the tail), we're about to overflow the buffer
	// and so we don't write the character or advance the head.
	while (i != buffer->tail) {
		int c;
		midiEventPacket_t event;
		if (!USB_Available(MIDI_RX)) {
#if defined(ARDUINO_ARCH_SAM)
			udd_ack_fifocon(MIDI_RX);
#endif
			//break;
		}
		c = USB_Recv(MIDI_RX, &event, sizeof(event) );

		//MIDI packet has to be 4 bytes
		if(c < 4)
			return;
		buffer->midiEvent[buffer->head] = event;
		buffer->head = i;

		i = (i + 1) % MIDI_BUFFER_SIZE;
	}
}

uint32_t MIDI_::available(void)
{
	
	ring_bufferMIDI *buffer = &midi_rx_buffer;
	return (uint32_t)(MIDI_BUFFER_SIZE + buffer->head - buffer->tail) % MIDI_BUFFER_SIZE;
}


midiEventPacket_t MIDI_::read(void)
{
        midiEventPacket_t c;
	ring_bufferMIDI   *buffer = &midi_rx_buffer;

	if(((uint32_t)(MIDI_BUFFER_SIZE + buffer->head - buffer->tail) % MIDI_BUFFER_SIZE) > 0) {
	    c = buffer->midiEvent[buffer->tail];
	} else {
            if (USB_Available(MIDI_RX)) {
	        accept();
	        c = buffer->midiEvent[buffer->tail];
	    } else {
	        c.header = 0;
	        c.byte1 = 0;
	        c.byte2 = 0;
	        c.byte3 = 0;
	    }
	}
	// if the head isn't ahead of the tail, we don't have any characters
	if (buffer->head != buffer->tail)
	{
		buffer->tail = (uint32_t)(buffer->tail + 1) % MIDI_BUFFER_SIZE;
	}
	return c;
}

void MIDI_::flush(void)
{
	USB_Flush(MIDI_TX);
}

size_t MIDI_::write(const uint8_t *buffer, size_t size)
{
	/* only try to send bytes if the high-level MIDI connection itself
	 is open (not just the pipe) - the OS should set lineState when the port
	 is opened and clear lineState when the port is closed.
	 bytes sent before the user opens the connection or after
	 the connection is closed are lost - just like with a UART. */

	// TODO - ZE - check behavior on different OSes and test what happens if an
	// open connection isn't broken cleanly (cable is yanked out, host dies
	// or locks up, or host virtual serial port hangs)

	// first, check the TX buffer to see if it's ready for writing.
	// USB_Send() may block if there's no one listening on the other end.
	// in that case, we don't want to block waiting for someone to connect,
	// because that would freeze the whole sketch
	// instead, we'll just drop the packets and hope the caller figures it out.
	if (is_write_enabled(MIDI_TX))
	{

		int r = USB_Send(MIDI_TX, buffer, size);

		if (r > 0)
		{
			return r;
		} else
		{
			return 0;
		}
	}
	return 0;
}

void MIDI_::sendMIDI(midiEventPacket_t event)
{
	uint8_t data[4];
	data[0] = event.header;
	data[1] = event.byte1;
	data[2] = event.byte2;
	data[3] = event.byte3;
	write(data, 4);
}

MIDI_::MIDI_(void) : PluggableUSBModule(2, 2, epType)
{
	epType[0] = EP_TYPE_BULK_OUT_MIDI;	// MIDI_ENDPOINT_OUT
	epType[1] = EP_TYPE_BULK_IN_MIDI;		// MIDI_ENDPOINT_IN
	PluggableUSB().plug(this);
}
