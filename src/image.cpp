#include "image.h"

Image::~Image()
{
	if(width != 0 && height != 0)
	{
		/*
		Scanline<RGBPixel<uint8_t>>* lines = (Scanline<RGBPixel<uint8_t>>*)imageData;
		for(int i = 0; i < height; i++)
		{
			delete[] lines[i].pixels;
		}
		delete[] lines;
		*/
		delete[] imageData;
	}
}
