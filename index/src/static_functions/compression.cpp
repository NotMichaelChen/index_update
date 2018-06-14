#include "compression.hpp"

#include <stdexcept>
#include <algorithm>

//Pads the given vector of bytes to the boundary of a given word length
void pad(std::vector<uint8_t>& data, unsigned int len) {
    size_t sizeincrease = len - (data.size() % len);
    if(sizeincrease != len)
        data.resize(data.size() + sizeincrease);
}

//Compresses a vector of posting data using the given compression method
//When delta encoding, do *not* assume field is already sorted
//TODO: Determine how to figure out padding length
std::vector<uint8_t> compress_block(std::vector<unsigned int>& field, std::list<uint8_t> encoder(unsigned int), bool delta) {
    std::vector<uint8_t> compressed;
    if(delta) {
        std::sort(field.begin(), field.end());
        std::vector<unsigned int> deltaencode;
        deltaencode.push_back(field[0]);

        for(size_t i = 1; i < field.size(); i++) {
            if(field[i] < field[i-1])
                throw std::invalid_argument("negative during delta compressing " + std::to_string(field[i-1]) + " " + std::to_string(field[i]) + "\n");
            deltaencode.push_back(field[i] - field[i-1]);
        }
        compressed = encode_array(deltaencode, encoder, 1);
    }
    else {
        compressed = encode_array(field, encoder, 1);
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

//Encodes an array of numbers using the given encoder function
std::vector<uint8_t> encode_array(std::vector<unsigned int>& nums, std::list<uint8_t> encoder(unsigned int), int padding) {
    std::list<uint8_t> bytestream;

    std::list<uint8_t> numbytes;
    for(const unsigned int &n : nums) {
        numbytes = encoder(n);
        //pad(bytes,padding);

        //numbytes is empty after this splice
        bytestream.splice(bytestream.end(), numbytes);
    }

    std::vector<uint8_t> result;
    result.reserve(bytestream.size());
    std::copy(std::begin(bytestream), std::end(bytestream), std::back_inserter(result));
    
    return result;
}