#include "reader.h"
#include "debug.h"
#include "PNGImage.h"
#include "zlib.h"
#include <cstdint>
#include <cstring>
#include <iostream>

using std::cout, std::endl;

int main()
{
	//ZLibInflator zlib;
	//zlib.test();

	PNGImage image("test.png");

	while(image.readNextChunk()){}
}
