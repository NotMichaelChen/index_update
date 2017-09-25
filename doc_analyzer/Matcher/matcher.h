#ifndef MATCHER_H
#define MATCHER_H

#include <vector>

#include "externalpostings.h"
#include "block.h"
#include "stringencoder.h"

namespace Matcher {
    //Gets the optimal set of common blocks of text between the two files
    std::vector<Block*> getOptimalBlocks(StringEncoder& se, int minblocksize, int maxblockcount, int selectionparameter);
    //Specifically generates postings given a vector of blocks
    //fragID refers to the next ID to use
    std::pair<std::vector<ExternNPposting>, std::vector<ExternPposting>>
        getPostings(std::vector<Block*>& commonblocks, unsigned int doc_id, unsigned int fragID, StringEncoder& se);
}
#endif