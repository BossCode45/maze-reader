#include "reader.h"

#include "debug.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>

Reader::Reader(std::string filename)
{
	file = fopen(filename.c_str(), "rb");
	refreshBuffer();
	ready = true;
}
Reader::~Reader()
{
	if(ready)
		fclose(file);
}

char Reader::readByte()
{
	if(pos == BUFFER_SIZE)
		refreshBuffer();
	return buffer[pos++];
}

void Reader::refreshBuffer()
{
		fread(buffer, sizeof(buffer), 1, file);
		pos = 0;
}

template<> uint8_t Reader::readData<uint8_t>()
{
	return readByte();
}

template<> uint16_t Reader::readData<uint16_t>()
{
	uint16_t num = 0;
	for(int i = 0; i < 2; i++)
	{
		num += readByte() << (8 * (1-i));
	}
	return num;
}

template<> uint32_t Reader::readData<uint32_t>()
{
	uint32_t num = 0;
	for(int i = 0; i < 4; i++)
	{
		uint8_t byte = readByte();
		DEBUG(std::cout << std::hex << 0+byte << " ");
		num += byte << (8 * (3-i));
	}
	DEBUG(std::cout << std::dec << std::endl);
	return num;
}

template<> uint64_t Reader::readData<uint64_t>()
{
	uint64_t num = 0;
	for(int i = 0; i < 8; i++)
	{
		num += readByte() << (8 * (7-i));
	}
	return num;
}

void Reader::readBytes(char* out, size_t len)
{
	while(len > 0)
	{
		size_t bytesToRead = std::min(len, BUFFER_SIZE - pos);
		if(bytesToRead == 0)
		{
			refreshBuffer();
			continue;
		}
		memcpy(out, buffer + pos, bytesToRead);
		out += bytesToRead;
		len -= bytesToRead;
		pos += bytesToRead;
	}
}

void Reader::skipBytes(size_t len)
{
	while(len > 0)
	{
		size_t bytesToRead = std::min(len, BUFFER_SIZE - pos);
		if(bytesToRead == 0)
		{
			refreshBuffer();
			continue;
		}
		len -= bytesToRead;
		pos += bytesToRead;
	}
}

void Reader::close()
{
	fclose(file);
	ready = false;
}
