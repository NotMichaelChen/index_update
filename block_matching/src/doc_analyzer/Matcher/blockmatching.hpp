#ifndef BLOCKMATCHING_HPP
#define BLOCKMATCHING_HPP

#include <vector>

#include "block.h"

//Extracts all potential common blocks between file versions that may be selected by the graph algorithm

//Gets all possible common blocks of text of size minsize. Extends blocks where necessary
//Blocks can be overlapping; this is fixed with the other two functions
std::vector<std::shared_ptr<Block>> getCommonBlocks(int minsize, StringEncoder& se);
//Extends common blocks, and removes blocks that are overlapped by the extended block
void extendBlocks(std::vector<std::shared_ptr<Block>>& allblocks, StringEncoder& se);
//Resolves blocks that may be only partially overlapping
//This is done by adding extra blocks that represent the overlaps
void resolveIntersections(std::vector<std::shared_ptr<Block>>& allblocks);

#endif