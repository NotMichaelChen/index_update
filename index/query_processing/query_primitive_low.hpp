#ifndef QUERY_PRIMITIVE_LOW_HPP
#define QUERY_PRIMITIVE_LOW_HPP

#include <vector>
#include <map>
#include <fstream>

#include "extended_lexicon.hpp"
#include "posting.hpp"
#include "global_parameters.hpp"

class query_primitive_low {
public:
    query_primitive_low(unsigned int termID, GlobalType::NonPosIndex& index);
    query_primitive_low(unsigned int termID, std::vector<mData>::iterator mdata);

    //advances the read pointer of lp to the posting with the smallest
    //docID that is at least x, and then returns that docID. Note that read pointers only move
    //forward; thus, if the pointer currently points to a posting with docID y > x, then the pointer
    //will not be moved, and y will be returned
    unsigned int nextGEQ(unsigned int pos, bool& failure);

    //getFreq() decompresses the block of frequency values associated with the current
    //posting, if it has not been decompressed yet, and returns the frequency value of the current
    //posting
    //NOTE: undefined if nextGEQ returned invalid
    unsigned int getFreq();

    //Gets the number of the index that qpl is pointing to
    //Returns -1 for inmemory index
    int getIndexNumber();

private:
    bool inmemory;
    
    //In-memory variables
    std::vector<nPosting> postinglist;
    size_t postingindex;

    //Metadata
    std::vector<mData>::iterator metadata;
    std::vector<unsigned int> last_docID;
    std::vector<unsigned int> blocksizes;

    //State

    //Which docID block we are in
    //Index of last_docID
    size_t docIDindex;
    //Which number in the block we are currently pointed to
    size_t blockindex;
    //File pointer position pointing to the beginning of a docID block
    long docblockpos;
    //Determines if frequency block has been decompressed/is valid
    bool freqdecompressed;

    //Vectors to hold decompressed blocks
    std::vector<unsigned int> docblock;
    std::vector<unsigned int> freqblock;

    //Filestream
    std::ifstream ifile;
};

#endif