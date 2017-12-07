#ifndef META_H
#define META_H

#include <string>

//Used in the extended lexicon and associated with a termID
struct mData{
    //For which file the metadata is for
    std::string filename;
    //Starting position of the inverted list
    long start_pos;
    //Where the blocks of compressed postings start
    long posting_offset;
    //Ending position of docIDs
    long docID_end;
    //Ending position of second field
    long second_end;
    //Where the posting count variable (and lastdocID array) is
    long postingCount_offset;
    //Where the array of block sizes begins
    long size_offset;
    //Where the inverted list ends
    long end_offset;
};

#endif
