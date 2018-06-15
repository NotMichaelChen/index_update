#ifndef BLOCKMATCHING_HPP
#define BLOCKMATCHING_HPP

#include <vector>

#include "block.h"

namespace Matcher
{

//Extracts all potential common blocks between file versions that may be selected by the graph algorithm

//Gets all possible common blocks of text of size minsize. Extends blocks where necessary
//Blocks can be overlapping; this is fixed with the other two functions
std::vector<Block> getCommonBlocks(int minsize, StringEncoder& se);
//Resolves blocks that may be only partially overlapping
//This is done by adding extra blocks that represent the overlaps
void resolveIntersections(std::vector<Block>& allblocks);

}
#endif