#include "zlib.h"

#include <bitset>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#define MAXBITS 15

using std::cout, std::endl;

void ZLibInflator::calculateCodes(uint8_t* lengths, uint16_t* codesOut, int codeCount)
{
	const int biggestLen = 15;

	uint16_t lenCounts[biggestLen + 1];
	memset(lenCounts, 0, (biggestLen + 1)*sizeof(uint16_t));
	for(int i = 0; i < codeCount; i++)
		lenCounts[lengths[i]]++;

	
	lenCounts[0] = 0;
	
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
		if(len == 0)
			continue;
		codesOut[i] = nextCodes[len]++;
	}
}

void ZLibInflator::buildHuffmanTree(uint8_t* lengths, uint16_t* codes, int codeCount)
{
	buildHuffmanTree(lengths, codes, codeCount, &tree);
}



void ZLibInflator::buildHuffmanTree(uint8_t* lengths, uint16_t* codes, int codeCount, HuffmanTree* tree)
{
	for(uint16_t i = 0; i < codeCount; i++)
	{
		uint16_t code = codes[i];
		uint8_t len = lengths[i];
		if(len == 0)
			continue;
		HuffmanTree* current = tree;
		for(int j = 0; j < len; j++)
		{
			bool right = code & (0b1 << (len - 1 - j));
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

void ZLibInflator::buildStaticHuffmanTree()
{
	if(staticTree)
		return;
	cout << "Building static tree" << endl;
	buildStaticHuffmanTree(&tree, &distTree);
	staticTree = true;
	haveTree = true;
}
void ZLibInflator::buildStaticHuffmanTree(HuffmanTree* treeOut, HuffmanTree* distTreeOut)
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
	buildHuffmanTree(lens, codes, codeCount, treeOut);
	
	uint8_t nDistCodes = 32;
	uint8_t distCodeLens[nDistCodes];
	uint16_t distCodes[nDistCodes];

	for(int i = 0; i < nDistCodes; i++)
		distCodeLens[i] = i;

	calculateCodes(distCodeLens, distCodes, nDistCodes);
	buildHuffmanTree(distCodeLens, distCodes, nDistCodes, distTreeOut);
}

void ZLibInflator::buildDynamicHuffmanTree(StreamData* stream)
{
	if(haveTree)
	{
		tree.free();
		distTree.free();
	}
	buildDynamicHuffmanTree(stream, &tree, &distTree);
	haveTree = true;
}
void ZLibInflator::buildDynamicHuffmanTree(StreamData* stream, HuffmanTree* treeOut, HuffmanTree* distTreeOut)
{
	unsigned int nLitCodes = nextBits(stream, 5) + 257;
	uint16_t litCodes[nLitCodes];

	unsigned int nDistCodes = nextBits(stream, 5) + 1;
	uint16_t distCodes[nDistCodes];

	uint8_t codeLens[nLitCodes + nDistCodes];
	
	uint8_t nLenCodes = nextBits(stream, 4) + 4;
	uint8_t lenCodeLens[19];
	memset(lenCodeLens,  0, sizeof(uint8_t)*19);
	uint16_t lenCodes[19];

	const static uint8_t lenCodeOrder[] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };
	
	for(int i = 0; i < nLenCodes; i++)
		lenCodeLens[lenCodeOrder[i]] = nextBits(stream, 3);

	calculateCodes(lenCodeLens, lenCodes, 19);
	HuffmanTree lenCodeTree;
	buildHuffmanTree(lenCodeLens, lenCodes, 19, &lenCodeTree);

	int i = 0;
	uint8_t extraBits[] = {2, 3, 7};
	uint8_t repStarts[] = {3, 3, 11};
	
	while(i < nLitCodes + nDistCodes)
	{
		uint16_t code = getNextCode(stream, &lenCodeTree);
		if(code < 16)
			codeLens[i++] = (uint8_t)code;
		else if(code < 19)
		{
			code -= 16;
			int reps = repStarts[code] + nextBits(stream, extraBits[code]);
			uint8_t len = 0;
			if(code == 0)
			{
				if(i == 0)
					cout << "Trying to repeat non existent value in dynamic huffman code creation" << endl;
				else
					len = codeLens[i - 1];
			}
			if(i + reps > nLitCodes + nDistCodes)
				cout << "other big errror oh no " << i << " " << 0+reps << endl;
			for(int j = 0; j < reps; j++)
			{
				codeLens[i++] = len;
			}
		}
		else
		{
			cout << "big error oh no" << endl;
		}
	}

	calculateCodes(codeLens, litCodes, nLitCodes);
	buildHuffmanTree(codeLens, litCodes, nLitCodes, treeOut);

	calculateCodes(&codeLens[nLitCodes], distCodes, nDistCodes);
	buildHuffmanTree(&codeLens[nLitCodes], distCodes, nDistCodes, distTreeOut);
}

void HuffmanTree::free()
{
	if(left != nullptr)
	{
		delete left;
		left = nullptr;
	}
	if(right != nullptr)
	{
		delete right;
		right = nullptr;
	}
	val = 0xFFFF;
}
HuffmanTree::~HuffmanTree()
{
	free();
}

uint16_t ZLibInflator::getNextCode(StreamData* stream)
{
	return getNextCode(stream, &tree);
}

uint16_t ZLibInflator::getNextCode(StreamData* stream, HuffmanTree* tree)
{
	while(tree->val == 0xFFFF)
	{
		bool right = nextBit(stream);

		if(tree->left == nullptr && !right)
			cout << "bad left" << endl;
		if(tree->right == nullptr && right)
			cout << "bad right" << endl;
		tree = (right)?tree->right:tree->left;
	}
	return tree->val;
}

bool ZLibInflator::nextBit(StreamData* stream)
{
	long bit = stream->pos % 8;
	long byte = stream->pos / 8;
	if(byte >= stream->length)
	{
		cout << byte << " " << stream->length << endl;
		throw std::out_of_range("Ran out of compressed data");
	}
	stream->pos++;
	//cout <<  ((stream->data[byte] & (0b1 << bit))?"1":"0");
	return (stream->data[byte] & (0b1 << bit))?0b1:0b0;
}

uint16_t ZLibInflator::nextBits(StreamData* stream, int bits)
{
	if(bits > 16)
		cout << "Too many bits(" << bits << ")!!!" << endl;
	uint16_t out = 0;
	for(int i = 0; i < bits; i++)
	{
		out |= nextBit(stream) << i;
	}
	return out;
}

int ZLibInflator::decodeData(uint8_t* data, unsigned long length, uint8_t* out, unsigned long outLength)
{
	staticTree = false;
	const unsigned int lenStart[] = { /* Size base for length codes 257..285 */
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
        35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
    const unsigned int lenExtra[] = { /* Extra bits for length codes 257..285 */
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
        3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};

	
    const unsigned int distStart[] = { /* Offset base for distance codes 0..29 */
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
        257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
        8193, 12289, 16385, 24577};
    const unsigned int distExtra[] = { /* Extra bits for distance codes 0..29 */
        0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
        7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
        12, 12, 13, 13};

	
	StreamData stream = { data, length, 0};
	long outPos = 0;
	
	bool final;
	do
	{

		final = nextBit(&stream);
		int method = nextBits(&stream, 2);

		//cout << (final?"Final chunk!\n":"") << "Compression method: " << method << endl;

		//cout << outPos << " " << outLength << endl;
		
		if(method == 1)
			buildStaticHuffmanTree();
		else if(method == 2)
			buildDynamicHuffmanTree(&stream);
		else if(method == 0)
		{
			while(stream.pos%8 != 0)
				stream.pos++;
			uint16_t LEN = nextBits(&stream, 16);
			uint16_t NLEN = nextBits(&stream, 16);
			NLEN++;
			if(LEN + NLEN != 0)
				throw std::invalid_argument("NLEN and LEN don't match");
			for(int i = 0; i < LEN; i++)
				out[outPos++] = nextBits(&stream, 8);
			continue;
		}
		else
		{
			cout << "Reserved???" << endl;
			return -1;
		}

		uint16_t code;
		do
		{
			code = getNextCode(&stream);
			if(outPos > outLength && code != 256)
			{
				throw std::out_of_range("No more space left in image (normal)");
			}
			if(code < 256)
				out[outPos++] = (uint8_t)code;
			else if(code > 256)
			{
				unsigned int len = lenStart[code-257] + (int)nextBits(&stream, lenExtra[code-257]);
				unsigned int distCode = getNextCode(&stream, &distTree);
				
				unsigned int dist = distStart[distCode] + (int)nextBits(&stream, distExtra[distCode]);
				if(outPos + len > outLength)
				{
					throw std::out_of_range("No more space left in image (RLE error)");
				}
				for(int i = 0; i < len; i++)
				{
					out[outPos] = out[outPos - dist];
					outPos++;
				}
			}
		}
		while(code != 256);
	}
	while(!final);

	return 0;
}
