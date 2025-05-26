#include "image.h"

Image::~Image()
{
	if(width != 0 && height != 0)
	{
		delete[] imageData;
	}
}

