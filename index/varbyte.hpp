#ifndef VARBYTE_H
#define VARBYTE_H

#include <vector>
#include <fstream>

std::vector<uint8_t> VBEncode(unsigned int num);
std::vector<uint8_t> VBEncode(std::vector<unsigned int>& nums);
std::vector<unsigned int> VBDecode(std::ifstream& ifile, long start_pos = 0, long end_pos = 0);
std::vector<unsigned int> VBDecode(std::vector<char>& vec);

#endif
