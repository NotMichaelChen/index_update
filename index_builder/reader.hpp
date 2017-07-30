#ifndef READER_H
#define READER_H
#include <fstream>
#include <vector>
#include <string>
#include <map>

class Reader{
public:

	std::vector<char> read_com(ifstream& infile);

	std::vector<unsigned int> VBDecode(ifstream& ifile, long start_pos = 0, long end_pos = ifile.tellg());

	std::vector<unsigned int> VBDecode(std::vector<char>& vec);

	std::vector<Posting> decompress(std::string filename, unsigned int termID, map<unsigned int, mData>& dict);
};

#endif
