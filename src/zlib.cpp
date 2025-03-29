#include "zlib.h"

#include <bitset>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>

using std::cout, std::endl;

void ZLibInflator::calculateCodes(uint8_t* lengths, uint16_t* codesOut, int codeCount)
{
	uint8_t biggestLen = 0;
	for(int i = 0; i < codeCount; i++)
		if(lengths[i] > biggestLen)
			biggestLen = lengths[i];

	uint16_t lenCounts[biggestLen + 1];
	memset(lenCounts, 0, (biggestLen + 1)*sizeof(uint16_t));
	for(int i = 0; i < codeCount; i++)
		lenCounts[lengths[i]]++;
	
	uint16_t nextCodes[biggestLen + 1];
	uint16_t code = 0;
	for(int bits = 1; bits < biggestLen + 1; bits++)
	{
		code = (code + lenCounts[bits - 1]) << 1;
		nextCodes[bits] = code;
	}

	for(int i = 0; i < codeCount; i++)
	{
		uint8_t len = lengths[i];
		codesOut[i] = nextCodes[len];
		nextCodes[len]++;
	}
}

void ZLibInflator::buildHuffmanTree(uint8_t* lengths, uint16_t* codes, int codeCount)
{
	for(uint16_t i = 0; i < codeCount; i++)
	{
		uint16_t code = codes[i];
		uint8_t len = lengths[i];
		HuffmanTree* current = &tree;
		for(int j = 0; j < len; j++)
		{
			bool right = (code >> (len-j-1)) & 0b1;
			if(right)
			{
				if(current->right != nullptr)
					current = current->right;
				else
				{
					current->right = new HuffmanTree();
					current = current->right;
				}
			}
			else
			{
				if(current->left != nullptr)
					current = current->left;
				else
				{
					current->left = new HuffmanTree();
					current = current->left;
				}
			}
		}
		current->val = i;
	}
}

HuffmanTree::~HuffmanTree()
{
	if(left != nullptr)
		delete left;
	if(right != nullptr)
		delete right;
}

uint16_t ZLibInflator::getNextCode(std::string stream, int* pos)
{
	HuffmanTree* current = &tree;
	while(current->val == 0xFFFF)
	{
		/*
		if(current->left == nullptr)
			cout << "bad left" << endl;
		if(current->right == nullptr)
			cout << "bad right" << endl;
		*/
		current = (stream.at((*pos)++) == '0')?current->left:current->right;
	}
	return current->val;
}

void ZLibInflator::test()
{
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

	calculateCodes(lens, codes, codeCount);

	uint16_t setCodes[] = {0, 143, 144, 255, 256, 279, 280, 287};
	for(int i = 0; i < 8; i++)
	{
		int code = setCodes[i];
		size_t start = 16-lens[code];
		std::string bitsetString = std::bitset<16>(codes[code]).to_string().substr(start);
		cout << std::setw(3) << code << ": "  << std::setw(9) <<std::left << bitsetString << endl;

		/*
		cout << "     ";
		for(int j = 0; j < lens[code]; j++)
		{
			cout << (((codes[code] >> (lens[code]-j-1)) & 0b1)?"1":"0");
		}
		cout << endl;
		*/
	}

	buildHuffmanTree(lens, codes, codeCount);
	std::string stream = "001011110111111";
	int pos = 0;
	cout << getNextCode(stream, &pos) << endl;
	cout << getNextCode(stream, &pos) << endl;
}
