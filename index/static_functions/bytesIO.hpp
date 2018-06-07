#ifndef BYTESIO_HPP
#define BYTESIO_HPP

#include <fstream>
#include <vector>

//Writes a given block (vector) of compressed posting data into the file
unsigned int writeBytesBlock(std::vector<uint8_t>& num, std::ofstream& ofile);
//Reads a block of raw compressed data from the file
std::vector<uint8_t> readBytesBlock(size_t buffersize, std::ifstream& ifile);

//Don't pass var by reference here to allow passing in expressions
template<typename T>
void writeAsBytes(T var, std::ofstream& ofile);

//Pass var by reference here to read into it
template<typename T>
void readFromBytes(T& var, std::ifstream& ifile);

#include "bytesIO.tcc"

#endif