#pragma once
#include <stdint.h>

typedef struct
{
	uint8_t header;
	uint8_t byte1;
	uint8_t byte2;
	uint8_t byte3;
} midiEventPacket_t;
