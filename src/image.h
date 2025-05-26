#pragma once

#include <cstddef>
#include <cstdint>

template <typename T>
struct Pixel
{
	T r, g, b, a;
};


class Image
{
protected:
	uint8_t* imageData;
	uint8_t colorValues;
	uint8_t bpp;
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

	template <typename T>
	Pixel<T> getPixel(unsigned int x, unsigned int y);
};


template <typename T>
Pixel<T> Image::getPixel(unsigned int x, unsigned int y)
{
	Pixel<T> pixel;

	pixel.r = (T)imageData[y * width * colorValues * bitDepth/8 + x * colorValues * bitDepth/8];
	pixel.g = (T)imageData[y * width * colorValues * bitDepth/8 + x * colorValues * bitDepth/8 + 1];
	pixel.b = (T)imageData[y * width * colorValues * bitDepth/8 + x * colorValues * bitDepth/8 + 2];

	if(colorValues == 4)
		pixel.a = imageData[y * width * colorValues * bitDepth/8 + x * colorValues * bitDepth/8 + 3];
	else
		pixel.a = 0;

	return pixel;
}
