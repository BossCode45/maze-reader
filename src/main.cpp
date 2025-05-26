#include "reader.h"
#include "debug.h"
#include "PNGImage.h"
#include "zlib.h"
#include <cstdint>
#include <cstring>
#include <iostream>

using std::cout, std::endl;

int main(int argc, char* argv[])
{
	//ZLibInflator zlib;
	//zlib.test();

	if(argc < 2)
	{
		cout << "usage: " << argv[0] << " <image>" << endl;
		return 1;
	}
	
	std::string filename = argv[1];

	PNGImage image(filename);

	while(image.readNextChunk()){}
}
