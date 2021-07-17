#include "stream.h"

uint16_t endianSwap(uint16_t value) {
	return (value >> 8) | (value << 8);
}

uint32_t endianSwap(uint32_t value) {
	uint32_t tmp = ((value << 8) & 0xFF00FF00) | ((value >> 8) & 0xFF00FF);
	return (tmp << 16) | (tmp >> 16);
}

uint64_t endianSwap(uint64_t value) {
	value = ((value & 0x00000000FFFFFFFFull) << 32) | ((value & 0xFFFFFFFF00000000ull) >> 32);
	value = ((value & 0x0000FFFF0000FFFFull) << 16) | ((value & 0xFFFF0000FFFF0000ull) >> 16);
	value = ((value & 0x00FF00FF00FF00FFull) << 8)  | ((value & 0xFF00FF00FF00FF00ull) >> 8);
	return value;
}

DataStream::DataStream(uint8_t *data_, size_t size, bool littleEndian) {

	this->LE = littleEndian;
	this->data = (uint8_t *)malloc(size);
	this->error = 0;
	this->max_off = size;
	this->offset = 0;
	memcpy(this->data, data_, size);

}

DataStream::DataStream(size_t size, bool littleEndian) {

	this->LE = littleEndian;
	this->data = (uint8_t *)calloc(1, size);
	this->error = 0;
	this->max_off = size;
	this->offset = 0;

}

DataStream::~DataStream() {

	if(this->data)
		free(this->data);

}

void DataStream::Expand(size_t sz) {
	this->data = (uint8_t*)realloc(this->data, this->max_off + sz);
	memset(this->data + this->max_off, 0, sz);
	this->max_off += sz;
}

uint8_t DataStream::Read_U8() {
	if(this->rangeChk(1)) {
		uint8_t c = *(uint8_t*)(this->data + this->offset);
		this->offset += 1;
		return c;
	} else {
		this->error = -1;
		return 0;
	}
}

void DataStream::Write_U8(uint8_t c) {
	if(this->rangeChk(1)) {
		*(uint8_t*)(this->data + this->offset) = c;
		this->offset += 1;
	} else {
		this->error = -1;
	}
}

uint16_t DataStream::Read_U16() {
	if(this->rangeChk(2)) {
		uint16_t c = *(uint16_t*)(this->data + this->offset);
		c = (this->LE ? endianSwap(c) : c);
		this->offset += 2;
		return c;
	} else {
		this->error = -1;
		return 0;
	}
}

void DataStream::Write_U16(uint16_t c) {
	if(this->rangeChk(2)) {
		c = (this->LE ? endianSwap(c) : c);
		*(uint16_t*)(this->data + this->offset) = c;
		this->offset += 2;
	} else {
		this->error = -1;
	}
}

uint32_t DataStream::Read_U32() {
	if(this->rangeChk(4)) {
		uint32_t c = *(uint32_t*)(this->data + this->offset);
		c = (this->LE ? endianSwap(c) : c);
		this->offset += 4;
		return c;
	} else {
		this->error = -1;
		return 0;
	}
}

void DataStream::Write_U32(uint32_t c) {
	if(this->rangeChk(4)) {
		c = (this->LE ? endianSwap(c) : c);
		*(uint32_t*)(this->data + this->offset) = c;
		this->offset += 4;
	} else {
		this->error = -1;
	}
}

uint8_t *DataStream::Read_Bytes() {
	uint16_t len = this->Read_U16();
	uint8_t *ret_c = (uint8_t *)calloc(1, len);
	if(this->rangeChk(len)) {
		memcpy((void*)ret_c, this->data + this->offset, len);
		this->offset += len;
		return ret_c;
	} else {
		this->error = -1;
		return 0;
	}
}

void DataStream::Write_Bytes(uint8_t *c, size_t sz) {
	this->Write_U16(sz);
	if(this->rangeChk(sz)) {
		memcpy(this->data + this->offset, c, sz);
		this->offset += sz;
	} else {
		this->error = -1;
	}
}

const char *DataStream::Read_C_Str() {
	uint16_t len = this->Read_U16();
	const char *ret_c = (const char*)calloc(1, len);
	if(this->rangeChk(len)) {
		memcpy((void*)ret_c, this->data + this->offset, len);
		this->offset += len;
		return ret_c;
	} else {
		this->error = -1;
		return 0;
	}
}

void DataStream::Write_C_Str(const char *c) {
	int len = strlen(c) + 1;
	this->Write_U16(len);
	if(this->rangeChk(len)) {
		memcpy(this->data + this->offset, c, len);
		this->offset += len;
	} else {
		this->error = -1;
	}
}