#pragma once
#include<Winsock2.h>//htons
#include <cstdint>//uint16_t
#define CRC_CCITT16			0x1021	

//CRCソースコードhttp://hatenaclang.blogspot.com/2011/03/crc.html
uint16_t calculateCRC16(const char* data, int length)
{
	int i, ii;
	uint16_t crc = 0xFFFF;
	for (i = 0; i<length; i++) {
		crc ^= (uint16_t)(*data++ << 8);
		for (ii = 0; ii<8; ii++) {
			if (crc & 0x8000) { crc <<= 1; crc ^= CRC_CCITT16; }
			else { crc <<= 1; }
		}
	}
	return htons(crc);
}