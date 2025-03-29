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
};

class ZLibInflator
{
private:
	HuffmanTree tree;
public:
	ZLibInflator() = default;
	~ZLibInflator() = default;

	void calculateCodes(uint8_t* lengths, uint16_t* codesOut, int codeCount);

	void buildHuffmanTree(uint8_t* lengths, uint16_t* codes, int codeCount);

	uint16_t getNextCode(std::string stream, int* pos);

	void test();
};
