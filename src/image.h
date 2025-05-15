#pragma once

#include <cstddef>
#include <cstdint>

template <typename T>
struct RGBPixel
{
	T r, g, b;
};

template <typename T>
struct RGBAPixel
{	T r, g, b, a;
};


/*
template <typename T>
union Scanline
{
	struct
	{
		uint8_t filterMethod;
		T* pixels;
	} data;
	uint8_t* raw;
};
*/

template <typename T>
union Scanline
{
	uint8_t filterMethod;
	T* pixels;
};

#include <cstdint>
class Image
{
protected:
	uint8_t* imageData;
public:
	Image() = default;
	~Image();
	
	uint32_t width = 0;
	uint32_t height = 0;
	uint8_t bitDepth;
	uint8_t colorType;
	uint8_t compressionMethod;
	uint8_t filterMethod;
	uint8_t interlaceMethod;
};
