#ifndef META_H
#define META_H

#include <string>

//Used in the extended lexicon and associated with a termID
struct mData{
    //For which file the metadata is for
    //Includes the whole filepath
    std::string filename;
    //Starting position of the inverted list
    long start_pos;
    //Starting position of the lastdocID array
    long last_docID;
    //Starting position of the blocksizes array
    long blocksizes;
    //Starting position of the postinglist blocks
    long postings_blocks;
    //Where the inverted list ends
    long end_offset;
};

//Shifts all of the entries in the metadata struct
//Does *not* change the filename, which must be done outside this function
mData shift_metadata(mData metadata, long shift);

#endif
