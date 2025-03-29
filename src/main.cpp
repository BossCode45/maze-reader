#include "reader.h"
#include "debug.h"
#include "PNGImage.h"
#include <cstdint>
#include <cstring>
#include <iostream>

using std::cout, std::endl;

int main()
{
	PNGImage image("test.png");

	while(image.readNextChunk()){}
}
