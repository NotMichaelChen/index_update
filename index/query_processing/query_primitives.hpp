#ifndef QUERY_PRIMITIVES_HPP
#define QUERY_PRIMITIVES_HPP

#include <vector>
#include <map>
#include <fstream>

#include "../extended_lexicon.hpp"
#include "../posting.hpp"

class query_primitive {
public:
    query_primitive(int termID, std::map<unsigned int, std::vector<nPosting>>& index);
    query_primitive(int termID, std::vector<mData>::iterator mdata);

private:
    bool inmemory;
    
    std::vector<nPosting> postinglist;

    std::vector<mData>::iterator metadata;
    std::vector<unsigned int> last_docID;
    std::vector<unsigned int> blocksizes;

    size_t blockindex;

    std::vector<unsigned int> docblock;
    std::vector<unsigned int> freqblock;
    std::ifstream ifile;
};

#endif