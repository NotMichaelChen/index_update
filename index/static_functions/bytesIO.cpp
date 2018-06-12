#include "bytesIO.hpp"

// Writes a vector of bytes to the given file stream
// Returns how many bytes it wrote to the stream
unsigned int writeBytesBlock(std::vector<uint8_t>& num, std::ofstream& ofile) {
    /* Write the compressed posting to file byte by byte. */
    unsigned int start = ofile.tellp();
    ofile.write(reinterpret_cast<const char*>(num.data()), num.size());
    unsigned int end = ofile.tellp();
    return end - start;
}

// Reads a vector of unsigned chars from the given file stream
// Assumes input stream is already at the correct place
std::vector<uint8_t> readBytesBlock(size_t buffersize, std::ifstream& ifile) {
    std::vector<char> buffer(buffersize);
    ifile.read(&buffer[0], buffersize);
    std::vector<uint8_t> unsignedbuffer = std::vector<uint8_t>(buffer.begin(), buffer.end());
    return unsignedbuffer;
}