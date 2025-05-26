#include "PNGImage.h"
#include "zlib.h"
#include "puff.h"
#include <bitset>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <exception>
#include <iostream>
#include <array>
#include <stdexcept>

using std::cout, std::endl;

PNGImage::PNGImage(std::string filename)
	:reader(filename)
	,idatData()
{
	//cout << "Reader good" << endl;
	REGISTER_CHUNK_READER(IHDR);
	REGISTER_CHUNK_READER(iCCP);
	REGISTER_CHUNK_READER(sRGB);
	REGISTER_CHUNK_READER(eXIf);
	REGISTER_CHUNK_READER(iDOT);
	REGISTER_CHUNK_READER(pHYs);
	REGISTER_CHUNK_READER(tIME);
	REGISTER_CHUNK_READER(tEXt);
	REGISTER_CHUNK_READER(IDAT);
	REGISTER_CHUNK_READER(IEND);

	//cout << "Chunk readers loaded" << endl;

	char signature[8];
	uint8_t expected[] = {137, 80, 78, 71, 13, 10, 26, 10};
	reader.readBytes(signature, 8);
	if(strncmp(signature, (char*)expected, 8) != 0)
		cout << "UH OH" << endl;

	//cout << "PNG image initialised" << endl;
}

bool PNGImage::readNextChunk()
{
	if(end)
		return false;
	uint32_t chunkSize = reader.readData<uint32_t>();
	
	char chunkType[4];
	reader.readBytes(chunkType, 4);
	std::string chunkName(chunkType, 4);
	cout << "-------------" << endl;
	cout << "|Chunk: " << chunkName << "|" << endl;
	cout << "-------------" << endl;

	if(chunkReaders.count(chunkName) == 0)
	{
		cout << "Chunk reader not found!!!" << endl;
		reader.skipBytes(chunkSize + 4);
		if(islower(chunkType[0]))
		{
			cout << "\tAble to skip chunk" << endl;
			return true;
		}
		cout << "\tFatal error" << endl;
		return false;
	}

	void(PNGImage::*chunkReader)(uint32_t chunkSize) = chunkReaders.find(chunkName)->second;
	(this->*chunkReader)(chunkSize);

	reader.skipBytes(4); // CRC
		
	return true;
}

DEFINE_CHUNK_READER(IHDR)
{
	width = reader.readData<uint32_t>();
	height = reader.readData<uint32_t>();
	bitDepth = reader.readData<uint8_t>();
	colorType = reader.readData<uint8_t>();
	compressionMethod = reader.readData<uint8_t>();
	filterMethod = reader.readData<uint8_t>();
	interlaceMethod = reader.readData<uint8_t>();
	cout << "Width: " << width << ", Height: " << height << ", Bit depth: " << 0+bitDepth << ", Color type: " << 0+colorType << ", Compression method: " << 0+compressionMethod << ", Filter method: " << 0+filterMethod << ", Interlace method: " << 0+interlaceMethod << endl;

	if(colorType != 2 && colorType != 6)
		throw std::invalid_argument("Only color types 2 and 6 are supported");

	switch(colorType)
	{
	case 2: colorValues = 3; break;
	case 6: colorValues = 4; break;
	}


	bpp = colorValues * (bitDepth/8);

	unsigned long imageDataSize = width * height * bpp;

	cout << "Assigning " << imageDataSize << " bytes for image" << endl;

	imageData = new uint8_t[imageDataSize];
	
/*
	Scanline<RGBPixel<uint8_t>>* lines = new Scanline<RGBPixel<uint8_t>> [height];
	for(int i = 0; i < height; i++)
	{
		lines[i].pixels = new RGBPixel<uint8_t>[width];
	}
	imageData = (uint8_t*)lines;
*/
}

DEFINE_CHUNK_READER(iCCP)
{
	cout << "!!! iCCP chunk reader not finished !!!" << endl;
	std::string profileName;
	char c = reader.readByte();
	chunkSize--;
	while(c != 0)
	{
		profileName.push_back(c);
		c = reader.readByte();
		chunkSize--;
	}
	cout << profileName << endl;
	uint8_t compresssionMethod = reader.readByte();
	chunkSize--;
	cout << 0+compresssionMethod << endl;
	uint8_t CMF = reader.readByte();
	uint8_t CM = CMF & 0b00001111;
	uint8_t CINFO = (CMF & 0b11110000) >> 4;
	chunkSize--;
	uint8_t FLG = reader.readByte();
	bool check = (CMF * 256 + FLG)%31 == 0;
	bool FDICT = FLG & 0b00100000;
	uint8_t FLEVEL = FLG & 0b11000000;
	chunkSize--;
	cout << std::bitset<4>(CM) << ", " << std::bitset<4>(CINFO) << ", " << (check?"Valid":"Failed checksum") << ", " << (FDICT?"Dict is present":"No dict present") << ", " << std::bitset<2>(FLEVEL) << endl;
	char compressedData[chunkSize - 4];
	reader.readBytes(compressedData, chunkSize - 4);

	const int compressedSize = chunkSize - 4;
	
	uint32_t checkValue = reader.readData<uint32_t>();

	//end = true;
}

DEFINE_CHUNK_READER(sRGB)
{
	renderingIntent = reader.readData<uint8_t>();
	cout << "Rendering intent: " << 0+renderingIntent << endl;
}

DEFINE_CHUNK_READER(eXIf)
{
	char endian[4];
	reader.readBytes(endian, 4);
	for(int i = 0; i < 2; i++)
	{
		cout << endian[i];
	}
	for(int i = 2; i < 4; i++)
	{
		cout << " " << 0+endian[i];
	}
	cout << endl;
	char rest[chunkSize - 4];
	reader.readBytes(rest, chunkSize - 4);
	cout << std::hex;
	for(int i = 0; i < chunkSize - 4; i++)
	{
		cout << 0+rest[i] << " ";
	}
	cout << std::dec << endl;
}

DEFINE_CHUNK_READER(iDOT)
{
	cout << "!!! Ignoring iDOT !!!" << endl;
	reader.skipBytes(chunkSize);
}

DEFINE_CHUNK_READER(pHYs)
{
	pixelsPerX = reader.readData<uint32_t>();
	pixelsPerY = reader.readData<uint32_t>();
	unit = reader.readData<uint8_t>();
	cout << "Pixels per unit (x): " << pixelsPerX << ", Pixels per unit (y): " << pixelsPerY << ", unit: " << 0+unit << endl;
}

DEFINE_CHUNK_READER(tIME)
{
	year = reader.readData<uint16_t>();
	month = reader.readData<uint8_t>();
	day = reader.readData<uint8_t>();
	hour = reader.readData<uint8_t>();
	minute = reader.readData<uint8_t>();
	second = reader.readData<uint8_t>();
	cout << "Image last modified: " << 0+hour << ":" << 0+minute << ":" << 0+second << " " << 0+day << "-" << 0+month << "-" << 0+year << endl;
}

DEFINE_CHUNK_READER(tEXt)
{
	
	std::string keyword;
	char c = reader.readByte();
	chunkSize--;
	while(c != 0)
	{
		keyword.push_back(c);
		c = reader.readByte();
		chunkSize--;
	}
	cout << keyword << endl;
	std::string textString;
	c = reader.readByte();
	chunkSize--;
	while(chunkSize > 0)
	{
		textString.push_back(c);
		c = reader.readByte();
		chunkSize--;
	}
	textString.push_back(c);
	cout << textString << endl;
}

DEFINE_CHUNK_READER(IDAT)
{
	if(idatData.size() == 0)
	{
		uint8_t CMF = reader.readByte();
		uint8_t CM = (CMF & 0b11110000) >> 4;
		uint8_t CINFO = CMF & 0b00001111;
		chunkSize--;
		uint8_t FLG = reader.readByte();
		bool check = (CMF * 256 + FLG)%31 == 0;
		bool FDICT = FLG & 0b00000100;
		uint8_t FLEVEL = FLG & 0b00000011;
		chunkSize--;
		cout << std::bitset<4>(CM) << ", " << std::bitset<4>(CINFO) << ", " << (check?"Valid":"Failed checksum") << ", " << (FDICT?"Dict is present":"No dict present") << ", " << std::bitset<2>(FLEVEL) << endl;
	}
	
	char compressedData[chunkSize];
	reader.readBytes(compressedData, chunkSize);
	for(int i = 0; i < chunkSize; i++)
		idatData.push_back(compressedData[i]);

	/*
	unsigned long compressedSize = chunkSize - 4;
	
	unsigned long imageDataSize = height * (width * 3 + 1);
	cout << zlib.decodeData((uint8_t*)compressedData, compressedSize, imageData, imageDataSize) << endl;
	//cout << (int)puff((unsigned char*)imageData, &imageDataSize, (const unsigned char*)compressedData, &compressedSize) << endl;
	*/
	
	//uint32_t checkValue = reader.readData<uint32_t>();

	//end = true;
}

uint8_t paethPredictor(uint8_t a, uint8_t b, uint8_t c)
{
	int p = a + b - c;
	int pa = abs(p - a);
	int pb = abs(p - b);
	int pc = abs(p - c);
	if (pa <= pb && pa <= pc)
		return a;
	else if (pb <= pc)
			return b;
	else
		return c;
}

DEFINE_CHUNK_READER(IEND)
{
	unsigned long imageDataSize = height * (width * bpp + 1);
	uint8_t* pngImageData = new uint8_t[imageDataSize];
	unsigned long idatSize = idatData.size();
	cout << "My inflate " << zlib.decodeData((uint8_t*)idatData.data(), idatSize, pngImageData, imageDataSize) << endl;
	end = true;
	reader.close();


	FILE* fd = fopen("tmp.bmp", "w");
	char magic[] = "BM";
	fwrite(magic, sizeof(char), 2, fd);
	uint32_t fileSize = 14 + 12 + width*height*/*(bitDepth/8)*/8*3;
	fwrite(&fileSize, sizeof(uint32_t), 1, fd);
	char zero[] = "\0\0\0\0";
	fwrite(zero, sizeof(char), 4, fd);
	uint32_t offset = 26;
	fwrite(&offset, sizeof(uint32_t), 1, fd);
	uint32_t headerSize = 12;
	fwrite(&headerSize, sizeof(uint32_t), 1, fd);
	uint16_t width = this->width;
	uint16_t height = this->height;
	uint16_t colorPlanes = 1;
	uint16_t bitsPerPixel = /*bitDepth*/8*3;
	fwrite(&width, sizeof(uint16_t), 1, fd);
	fwrite(&height, sizeof(uint16_t), 1, fd);
	fwrite(&colorPlanes, sizeof(uint16_t), 1, fd);
	fwrite(&bitsPerPixel, sizeof(uint16_t), 1, fd);

#define imageDataIndex(x, y) imageData[y*width*bpp + x]
#define pngImageDataIndex(x, y) pngImageData[y*(width*bpp + 1) + x + 1]
#define filterByte(y) pngImageDataIndex(-1, y)

	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width*bpp; x++)
		{
			if(filterByte(y) == 0)
			{
				imageDataIndex(x, y) = pngImageDataIndex(x, y);
			}
			else if(filterByte(y) == 1)
			{
				uint8_t sub = pngImageDataIndex(x, y);
				uint8_t raw = (x>=bpp)?imageDataIndex((x-bpp), y):0;
				imageDataIndex(x, y) = sub + raw;
			}
			else if(filterByte(y) == 2)
			{
				uint8_t up = pngImageDataIndex(x, y);
				uint8_t prior = (y>=1)?imageDataIndex(x, (y-1)):0;
				imageDataIndex(x, y) = up + prior;
			}
			else if(filterByte(y) == 4)
			{
				uint8_t a = (x>=bpp)?imageDataIndex((x-bpp), y):0;
				uint8_t b = (y>=1)?imageDataIndex(x, (y-1)):0;
				uint8_t c = (x>=bpp && y>=1)?imageDataIndex((x-bpp), (y-1)):0;
				uint8_t paeth = pngImageDataIndex(x, y);
				uint8_t predictor = paethPredictor(a, b, c);
				imageDataIndex(x, y) = paeth + predictor;
			}
			else
			{
				cout << "No method for filter type: " << (int)filterByte(y) << ", row: " << y << endl;
				throw "uh oh";
			}
		}
	}

#undef imageDataIndex
#undef pngImageDataIndex
#undef filterByte
	
	for(int y = height-1; y >= 0; y--)
	{
		for(int x = 0; x < width; x++)
		{
			Pixel<uint8_t> pixel = getPixel<uint8_t>(x, y);
			fwrite(&pixel.b, bitDepth/8, 1, fd);
			fwrite(&pixel.g, bitDepth/8, 1, fd);
			fwrite(&pixel.r, bitDepth/8, 1, fd);
		}
	}

	delete [] pngImageData;
	fclose(fd);
}
