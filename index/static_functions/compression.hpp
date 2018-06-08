#ifndef COMPRESSION_HPP
#define COMPRESSION_HPP

#include <vector>
#include <list>
#include <cstdint>

// Contains functions related to decompressing and compressing blocks of numbers

//Compresses a vector of posting data using the given compression method
std::vector<uint8_t> compress_block(std::vector<unsigned int>& field, std::list<uint8_t> encoder(unsigned int), bool delta);
//Decompresses a vector of posting data using the given decompression method
std::vector<unsigned int> decompress_block(std::vector<uint8_t>& block, std::vector<unsigned int> decoder(std::vector<uint8_t>&), bool delta);

//Encodes an array of numbers using the given encoder function
std::vector<uint8_t> encode_array(std::vector<unsigned int>& nums, std::list<uint8_t> encoder(unsigned int), int padding);

#endif