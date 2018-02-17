#include "compression.hpp"

#include <string>
#include <stdexcept>

//Compresses a vector of posting data using the given compression method
//When delta encoding, assume field is already sorted
std::vector<uint8_t> compress_block(std::vector<unsigned int>& field, std::vector<uint8_t> encoder(std::vector<unsigned int>&), bool delta) {
    std::vector<uint8_t> compressed;
    if(delta) {
        std::vector<unsigned int> deltaencode;
        deltaencode.push_back(field[0]);

        for(size_t i = 1; i < field.size(); i++) {
            if(field[i] < field[i-1])
                throw std::invalid_argument("negative during delta compressing " + std::to_string(field[i-1]) + " " + std::to_string(field[i]) + "\n");
            deltaencode.push_back(field[i] - field[i-1]);
        }
        compressed = encoder(deltaencode);
    }
    else {
        compressed = encoder(field);
    }
    return compressed;
}

//Decompresses a vector of posting data using the given decompression method
std::vector<unsigned int> decompress_block(std::vector<uint8_t>& block, std::vector<unsigned int> decoder(std::vector<uint8_t>&), bool delta) {
    std::vector<unsigned int> decompressed;
    decompressed = decoder(block);
    if(decompressed.size() == 0) {
        throw std::invalid_argument("Error, decompress_block final size is zero");
    }
    if(delta) {
        std::vector<unsigned int> undelta(decompressed.size());
        undelta[0] = decompressed[0];

        for(size_t i = 1; i < decompressed.size(); i++) {
            undelta[i] = undelta[i-1] + decompressed[i];
        }
        return undelta;
    }
    return decompressed;
}