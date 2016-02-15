//================================================================================
//================================================================================
//	MIDI USB class

#ifndef MIDIUSB_h
#define MIDIUSB_h

#include <stdint.h>
#include <Arduino.h>

#if ARDUINO < 10606
#error MIDIUSB requires Arduino IDE 1.6.6 or greater. Please update your IDE.
#endif

#if !defined(USBCON)
#error MIDIUSB can only be used with an USB MCU.
#endif

typedef struct
{
	uint8_t header;
	uint8_t byte1;
	uint8_t byte2;
	uint8_t byte3;
}midiEventPacket_t;

#if defined(ARDUINO_ARCH_AVR)

#include "PluggableUSB.h"

#define EPTYPE_DESCRIPTOR_SIZE		uint8_t
#define EP_TYPE_BULK_IN_MIDI 		EP_TYPE_BULK_IN
#define EP_TYPE_BULK_OUT_MIDI 		EP_TYPE_BULK_OUT
#define MIDI_BUFFER_SIZE			USB_EP_SIZE
#define is_write_enabled(x)			(1)

#elif defined(ARDUINO_ARCH_SAM)

#include "USB/PluggableUSB.h"

#define EPTYPE_DESCRIPTOR_SIZE		uint32_t
#define EP_TYPE_BULK_IN_MIDI		(UOTGHS_DEVEPTCFG_EPSIZE_512_BYTE | \
									UOTGHS_DEVEPTCFG_EPDIR_IN |         \
									UOTGHS_DEVEPTCFG_EPTYPE_BLK |       \
									UOTGHS_DEVEPTCFG_EPBK_1_BANK |      \
									UOTGHS_DEVEPTCFG_NBTRANS_1_TRANS |  \
									UOTGHS_DEVEPTCFG_ALLOC)
#define EP_TYPE_BULK_OUT_MIDI       (UOTGHS_DEVEPTCFG_EPSIZE_512_BYTE | \
									UOTGHS_DEVEPTCFG_EPTYPE_BLK |       \
									UOTGHS_DEVEPTCFG_EPBK_1_BANK |      \
									UOTGHS_DEVEPTCFG_NBTRANS_1_TRANS |  \
									UOTGHS_DEVEPTCFG_ALLOC)
#define MIDI_BUFFER_SIZE			EPX_SIZE
#define USB_SendControl				USBD_SendControl
#define USB_Available				USBD_Available
#define USB_Recv					USBD_Recv
#define USB_Send					USBD_Send
#define USB_Flush					USBD_Flush
#define is_write_enabled(x)			Is_udd_write_enabled(x)

#elif defined(ARDUINO_ARCH_SAMD)

#include "USB/PluggableUSB.h"

#define EPTYPE_DESCRIPTOR_SIZE		uint32_t
#define EP_TYPE_BULK_IN_MIDI 		USB_ENDPOINT_TYPE_BULK | USB_ENDPOINT_IN(0);
#define EP_TYPE_BULK_OUT_MIDI 		USB_ENDPOINT_TYPE_BULK | USB_ENDPOINT_OUT(0);
#define MIDI_BUFFER_SIZE			EPX_SIZE
#define USB_SendControl				USBDevice.sendControl
#define USB_Available				USBDevice.available
#define USB_Recv					USBDevice.recv
#define USB_Send					USBDevice.send
#define USB_Flush					USBDevice.flush
#define is_write_enabled(x)			(1)

#else

#error "Unsupported architecture"

#endif

#define MIDI_AUDIO								0x01
#define MIDI_AUDIO_CONTROL						0x01
#define MIDI_CS_INTERFACE						0x24
#define MIDI_CS_ENDPOINT						0x25
#define MIDI_STREAMING							0x3
#define MIDI_JACK_EMD							0x01
#define MIDI_JACK_EXT							0X02

_Pragma("pack(1)")

typedef struct
{
	uint8_t len;		// 9
	uint8_t dtype;		// 4
	uint8_t dsubType;
	uint16_t bcdADc;
	uint16_t wTotalLength;
	uint8_t bInCollection;
	uint8_t interfaceNumbers;
} MIDI_ACInterfaceDescriptor;

typedef struct
{
	uint8_t len;		// 9
	uint8_t dtype;		// 4
	uint8_t dsubType;
	uint8_t jackType;
	uint8_t jackID;
	uint8_t jackStrIndex;
} MIDIJackinDescriptor;

typedef struct
{
	uint8_t len;		// 9
	uint8_t dtype;		// 4
	uint8_t dsubType;
	uint8_t jackType;
	uint8_t jackID;
	uint8_t nPins;
	uint8_t srcJackID;
	uint8_t srcPinID;
	uint8_t jackStrIndex;
} MIDIJackOutDescriptor;

typedef struct
{
	EndpointDescriptor len;		// 9
	uint8_t refresh;		// 4
	uint8_t sync;
} MIDI_EPDescriptor;

typedef struct
{
	uint8_t len;		// 5
	uint8_t dtype;		// 0x24
	uint8_t subtype;
	uint8_t embJacks;
	uint8_t jackID;
} MIDI_EP_ACDescriptor;

typedef struct
{
	uint8_t len;		// 9
	uint8_t dtype;		// 4
	uint8_t dsubType;
	uint16_t bcdADc;
	uint16_t wTotalLength;
} MIDI_ASInterfaceDescriptor;

typedef struct
{
	//	IAD
	IADDescriptor                      iad;
	// MIDI Audio Control Interface
	InterfaceDescriptor                Audio_ControlInterface;
	MIDI_ACInterfaceDescriptor         Audio_ControlInterface_SPC;

	// MIDI Audio Streaming Interface
	InterfaceDescriptor                Audio_StreamInterface;
	MIDI_ASInterfaceDescriptor         Audio_StreamInterface_SPC;

	MIDIJackinDescriptor               MIDI_In_Jack_Emb;
	MIDIJackinDescriptor               MIDI_In_Jack_Ext;
	MIDIJackOutDescriptor              MIDI_Out_Jack_Emb;
	MIDIJackOutDescriptor              MIDI_Out_Jack_Ext;

	MIDI_EPDescriptor                  MIDI_In_Jack_Endpoint;
	MIDI_EP_ACDescriptor               MIDI_In_Jack_Endpoint_SPC;
	MIDI_EPDescriptor                  MIDI_Out_Jack_Endpoint;
	MIDI_EP_ACDescriptor               MIDI_Out_Jack_Endpoint_SPC;
} MIDIDescriptor;

#define D_AC_INTERFACE(_streamingInterfaces, _MIDIInterface) \
	{ 9, MIDI_CS_INTERFACE, 0x1, 0x0100, 0x0009, _streamingInterfaces, (uint8_t)(_MIDIInterface) }

#define D_AS_INTERFACE \
	{ 0x7, MIDI_CS_INTERFACE, 0x01,0x0100, 0x0041}

#define D_MIDI_INJACK(jackProp, _jackID) \
	{ 0x06, MIDI_CS_INTERFACE, 0x02, jackProp, _jackID, 0  }

#define D_MIDI_OUTJACK(jackProp, _jackID, _nPins, _srcID, _srcPin) \
	{ 0x09, MIDI_CS_INTERFACE, 0x3, jackProp, _jackID, _nPins, _srcID, _srcPin, 0  }

#define D_MIDI_JACK_EP(_addr,_attr,_packetSize) \
	{ 9, 5, _addr,_attr,_packetSize, 0, 0, 0}

#define D_MIDI_AC_JACK_EP(_nMIDI, _iDMIDI) \
	{ 5, MIDI_CS_ENDPOINT, 0x1, _nMIDI, _iDMIDI}

#define D_CDCCS(_subtype,_d0,_d1)	{ 5, 0x24, _subtype, _d0, _d1 }
#define D_CDCCS4(_subtype,_d0)		{ 4, 0x24, _subtype, _d0 }

_Pragma("pack()")

#define WEAK __attribute__ ((weak))

class MIDI_ : public PluggableUSBModule
{
// private:
// 	RingBuffer *_midi_rx_buffer;
private:
	void accept(void);
	EPTYPE_DESCRIPTOR_SIZE epType[2];

protected:
  // Implementation of the PUSBListNode
  int getInterface(uint8_t* interfaceNum);
  int getDescriptor(USBSetup& setup);
  bool setup(USBSetup& setup);
  uint8_t getShortName(char* name);

public:
	MIDI_(void);
	uint32_t available(void);
	midiEventPacket_t read(void);
	void flush(void);
	void sendMIDI(midiEventPacket_t event);
	size_t write(const uint8_t *buffer, size_t size);
	operator bool();
};
extern MIDI_ MidiUSB;

#endif	/* MIDIUSB_h */
