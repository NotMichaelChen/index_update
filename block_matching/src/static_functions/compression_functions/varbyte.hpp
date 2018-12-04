#ifndef VARBYTE_H
#define VARBYTE_H

#include <vector>
#include <list>
#include <cstdint>

std::list<uint8_t> VBEncode(unsigned int num);
std::vector<unsigned int> VBDecode(std::vector<uint8_t>& bytestream);

#endif
