#ifndef COMPRESSION_HPP
#define COMPRESSION_HPP

#include <vector>
#include <cstdint>

// Contains functions related to decompressing and compressing blocks of numbers

//Compresses a vector of posting data using the given compression method
std::vector<uint8_t> compress_block(std::vector<unsigned int>& field, std::vector<uint8_t> encoder(std::vector<unsigned int>&), bool delta);
//Decompresses a vector of posting data using the given decompression method
std::vector<unsigned int> decompress_block(std::vector<uint8_t>& block, std::vector<unsigned int> decoder(std::vector<uint8_t>&), bool delta);

#endif