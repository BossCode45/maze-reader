#pragma once

#include <cstdint>
#include <string>

struct HuffmanTree
{
	uint16_t val = 0xFFFF;
	HuffmanTree* left = nullptr; // 0
	HuffmanTree* right = nullptr; // 1
	HuffmanTree() = default;
	~HuffmanTree();
	void free();
};

struct StreamData
{
	uint8_t* data;
	unsigned long length;
	unsigned long pos;
};

class ZLibInflator
{
private:
	HuffmanTree tree;
	HuffmanTree distTree;
	bool staticTree = false;
	bool haveTree = false;
public:
	ZLibInflator() = default;
	~ZLibInflator() = default;

	static bool nextBit(StreamData* stream);
	static uint16_t nextBits(StreamData* stream, int bits); // Max 16 bits

	void calculateCodes(uint8_t* lengths, uint16_t* codesOut, int codeCount);

	void buildHuffmanTree(uint8_t* lengths, uint16_t* codes, int codeCount);
	void buildHuffmanTree(uint8_t* lengths, uint16_t* codes, int codeCount, HuffmanTree* treeOut);

	void buildStaticHuffmanTree();
	void buildStaticHuffmanTree(HuffmanTree* treeOut, HuffmanTree* distTreeOut);

	void buildDynamicHuffmanTree(StreamData* stream);
	void buildDynamicHuffmanTree(StreamData* stream, HuffmanTree* treeOut, HuffmanTree* distTreeOut);

	uint16_t getNextCode(StreamData* stream);
	uint16_t getNextCode(StreamData* stream, HuffmanTree* tree);

	int decodeData(uint8_t* data, unsigned long length, uint8_t* out, unsigned long outLength);
};
