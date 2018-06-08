#ifndef POSTINGIO_HPP
#define POSTINGIO_HPP

#include <vector>
#include <string>
#include <fstream>

#include "posting.hpp"

// Contains functions related to reading and writing from static_indexes

//STATIC INDEX LAYOUT (per posting list)
//termID, length in bytes, length in postings, compression methods (2 or 3 numbers), lennextblock (bytes), [lastdocID] (of each block),
//lennextblock (bytes), [blocksizes] (in bytes), lennextblock (bytes), [blocks]

//Reads a block of compressed data and decompressed it
std::vector<unsigned int> read_block(size_t buffersize, std::ifstream& ifile, std::vector<unsigned int> decoder(std::vector<uint8_t>&), bool delta);

//Writes a posting list to disk with compression
template <typename T>
void write_postinglist(std::ofstream& ofile, unsigned int termID, std::vector<T>& postinglist, bool positional);

//Reads a posting list from disk
//Assumes file stream is pointing to the unsigned int after termID
std::vector<Posting> read_pos_postinglist(std::ifstream& ifile, unsigned int termID);
std::vector<nPosting> read_nonpos_postinglist(std::ifstream& ifile, unsigned int termID);

#endif