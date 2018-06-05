#ifndef POSTINGIO_HPP
#define POSTINGIO_HPP

#include <vector>
#include <string>
#include <fstream>

#include "posting.hpp"
#include "meta.hpp"
#include "extended_lexicon.hpp"

// Contains functions related to reading and writing from static_indexes

//STATIC INDEX LAYOUT (per posting list)
//termID, length in bytes, compression methods (2 or 3 numbers), lennextblock (bytes), [lastdocID] (of each block),
//lennextblock (bytes), [blocksizes] (in bytes), lennextblock (bytes), [blocks]

//Compressed a block (vector) of ints and writes it to the file
unsigned int write_block(std::vector<unsigned int>& block, std::ofstream& ofile, std::vector<uint8_t> encoder(unsigned int), bool delta);

//Reads a block of compressed data and decompressed it
std::vector<unsigned int> read_block(size_t buffersize, std::ifstream& ifile, std::vector<unsigned int> decoder(std::vector<uint8_t>&), bool delta);

//Writes a given block (vector) of compressed posting data into the file
unsigned int write_raw_block(std::vector<uint8_t>& num, std::ofstream& ofile);

//Reads a block of raw compressed data from the file
std::vector<uint8_t> read_raw_block(size_t buffersize, std::ifstream& ifile);

//Writes a posting list to disk with compression
template <typename T>
void write_postinglist(std::ofstream& ofile, std::string& filepath, unsigned int termID, std::vector<T>& postinglist, bool positional);

//Reads a posting list from disk
std::vector<Posting> read_pos_postinglist(std::ifstream& ifile, unsigned int termID);
std::vector<nPosting> read_nonpos_postinglist(std::ifstream& ifile, unsigned int termID);

#endif