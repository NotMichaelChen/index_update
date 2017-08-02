#ifndef MATCHER_H
#define MATCHER_H

#include <vector>

#include "postings.h"
#include "block.h"
#include "stringencoder.h"

//Gets the optimal set of common blocks of text between the two files
std::vector<Block*> getCommonBlocks(StringEncoder& se, int minblocksize, int maxblockcount, int selectionparameter);
//Specifically generates postings given a vector of blocks
//fragID refers to the next ID to use
std::pair<std::vector<NonPositionalPosting>, std::vector<PositionalPosting>>
    getPostings(std::vector<Block*>& commonblocks, unsigned int doc_id, unsigned int fragID, StringEncoder& se);
    
#endif