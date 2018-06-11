#include "varbyte.hpp"

#include <bitset>
#include <math.h>
#include <algorithm>

//https://nlp.stanford.edu/IR-book/html/htmledition/variable-byte-codes-1.html

std::list<uint8_t> VBEncode(unsigned int num) {
    std::list<uint8_t> bytes;

    bytes.push_front(num % 128);
    while(num >= 128) {
        num /= 128;
        bytes.push_front(num % 128);
    }
    bytes.back() += 128;
    return bytes;
}

std::vector<unsigned int> VBDecode(std::vector<uint8_t>& bytestream) {
    std::vector<unsigned int> numbers;
    int n = 0;

    for(size_t i = 0; i < bytestream.size(); i++) {
        if(bytestream[i] < 128) {
            n = 128 * n + bytestream[i];
        }
        else {
            n = 128 * n + (bytestream[i] - 128);
            numbers.push_back(n);
            n = 0;
        }
    }
    return numbers;
}