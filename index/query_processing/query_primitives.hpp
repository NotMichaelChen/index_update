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

    //advances the read pointer of lp to the posting with the smallest
    //docID that is at least x, and then returns that docID. Note that read pointers only move
    //forward; thus, if the pointer currently points to a posting with docID y > x, then the pointer
    //will not be moved, and y will be returned
    unsigned int nextGEQ(unsigned int pos);

private:
    bool inmemory;
    
    std::vector<nPosting> postinglist;
    size_t postingindex;

    std::vector<mData>::iterator metadata;
    std::vector<unsigned int> last_docID;
    std::vector<unsigned int> blocksizes;

    //Which docID block to examine next
    size_t docIDindex;
    //Which number in the block we are currently pointed to
    size_t blockindex;

    std::vector<unsigned int> docblock;
    std::vector<unsigned int> freqblock;
    std::ifstream ifile;
};

#endif