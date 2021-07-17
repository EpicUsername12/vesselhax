#ifndef UDPEX_UDP_STREAM
#define UDPEX_UDP_STREAM

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <time.h>

uint16_t endianSwap(uint16_t value);
uint32_t endianSwap(uint32_t value);
uint64_t endianSwap(uint64_t value);

typedef struct Buffer {
	uint8_t *buf;
	size_t sz;
	void *arg;
} Buffer;

class DataStream {

public:
	uint8_t *data = NULL;
	int max_off = 0;
	int offset = 0;
	int error = 0;
	bool LE = false;

    DataStream(uint8_t *data_, size_t size, bool littleEndian = false);
    DataStream(size_t size, bool littleEndian = false);
    ~DataStream();

	void Expand(size_t sz);

	uint8_t Read_U8();
	uint16_t Read_U16();
	uint32_t Read_U32();
	uint8_t *Read_Bytes();
	const char *Read_C_Str();

	void Write_U8(uint8_t c);
	void Write_U16(uint16_t c);
	void Write_U32(uint32_t c);
	void Write_Bytes(uint8_t *c, size_t sz);
	void Write_C_Str(const char *c);

	bool rangeChk(int size) {
		return ((this->offset + size) <= this->max_off);
	}

};

#endif