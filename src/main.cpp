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

	std::string bits = "10100100100101011010001110100100";
	int pos = 0;

	ZLibInflator zlib;
	const int codeCount = 288;
	uint8_t lens[codeCount];
	uint16_t codes[codeCount];
	for(int i = 0; i < codeCount; i++)
	{
		if(i < 144)
			lens[i] = 8;
		else if(i < 256)
			lens[i] = 9;
		else if(i < 280)
			lens[i] = 7;
		else if(i < 288)
			lens[i] = 8;
	}
	zlib.calculateCodes(lens, codes, codeCount);
	zlib.buildHuffmanTree(lens, codes, codeCount);
	
	cout << (char)zlib.getNextCode(bits, &pos) << endl;
	cout << (char)zlib.getNextCode(bits, &pos) << endl;
	cout << (char)zlib.getNextCode(bits, &pos) << endl;
	cout << (char)zlib.getNextCode(bits, &pos) << endl;

	return 0;
	
	PNGImage image("test.png");

	while(image.readNextChunk()){}
}
