#include "PNGImage.h"
#include "zlib.h"
#include "puff.h"
#include <bitset>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <array>

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


	imageData = new uint8_t[height * (width * 3 + 1)];
	
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
	
	char compressedData[chunkSize - 4];
	cout << chunkSize - 4 << endl;
	reader.readBytes(compressedData, chunkSize - 4);
	for(int i = 0; i < chunkSize - 4; i++)
		idatData.push_back(compressedData[i]);

	/*
	unsigned long compressedSize = chunkSize - 4;
	
	unsigned long imageDataSize = height * (width * 3 + 1);
	cout << zlib.decodeData((uint8_t*)compressedData, compressedSize, imageData, imageDataSize) << endl;
	//cout << (int)puff((unsigned char*)imageData, &imageDataSize, (const unsigned char*)compressedData, &compressedSize) << endl;
	*/
	
	uint32_t checkValue = reader.readData<uint32_t>();

	//end = true;
}

DEFINE_CHUNK_READER(IEND)
{
	unsigned long imageDataSize = height * (width * 3 + 1);
	unsigned long idatSize = idatData.size();
	//cout << (int)puff((unsigned char*)imageData, &imageDataSize, idatData.data(), &idatSize) << endl;
	cout << zlib.decodeData(idatData.data(), idatData.size(), imageData, imageDataSize) << endl;
	end = true;
	reader.close();
}
