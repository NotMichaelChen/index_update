#ifndef VARBYTE_H
#define VARBYTE_H

#include <vector>
#include <fstream>

std::vector<uint8_t> VBEncode(unsigned int num);
std::vector<uint8_t> VBEncode(std::vector<unsigned int>& nums);
std::vector<unsigned int> VBDecode(std::vector<uint8_t>& bytestream);
std::vector<unsigned int> VBDecode(char* buffer, int length);

#endif
