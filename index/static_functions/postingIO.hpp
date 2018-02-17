#ifndef POSTINGIO_HPP
#define POSTINGIO_HPP

#include <vector>
#include <string>
#include <fstream>

#include "../posting.hpp"
#include "../meta.hpp"
#include "../extended_lexicon.hpp"

// Contains functions related to reading and writing from static_indexes

//Writes a given block (vector) of compressed posting data into the file
template <typename T>
unsigned int write_block(std::vector<T>& num, std::ofstream& ofile);

//Writes a posting list to disk with compression
template <typename T>
void write_postinglist(std::ofstream& ofile, std::string& filepath, unsigned int termID, std::vector<T>& postinglist, ExtendedLexicon& exlex, bool positional);

//Reads a posting list from disk
std::vector<Posting> read_pos_postinglist(std::ifstream& ifile, std::vector<mData>::iterator metadata, unsigned int termID);
std::vector<nPosting> read_nonpos_postinglist(std::ifstream& ifile, std::vector<mData>::iterator metadata, unsigned int termID);

#endif