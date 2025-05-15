#pragma once

#include <cstddef>
#include <cstdio>
#include <string>

#define BUFFER_SIZE 1024

class Reader
{
public:
	//Bytes are big endian
	Reader(std::string file);
	~Reader();

	template <typename T>
		T readData();

	char readByte();

	void readBytes(char* out, size_t len);

	void skipBytes(size_t len);

	void close();
private:
	char buffer[BUFFER_SIZE];
	size_t pos;
	FILE* file;
	bool ready = false;

	void refreshBuffer();
};
