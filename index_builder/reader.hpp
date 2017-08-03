#ifndef READER_H
#define READER_H
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include "posting.hpp"
#include "meta.hpp"

class Reader{
public:

	static std::vector<char> read_com(std::ifstream& infile, long end_pos);

	static std::vector<unsigned int> VBDecode(std::ifstream& ifile, long start_pos = 0, long end_pos = 0);

	static std::vector<unsigned int> VBDecode(std::vector<char>& vec);

	static std::vector<Posting> decompress_p(std::string namebase, unsigned int termID, std::map<unsigned int, std::pair<std::vector<mData>, std::vector<mDatanp>>>& dict);

	std::vector<nPosting> decompress_np(std::string namebase, unsigned int termID, std::map<unsigned int, std::pair<std::vector<mData>, std::vector<mDatanp>>>& dict);
};

#endif
