#ifndef BLOCK_H
#define BLOCK_H

#include <vector>
#include <iostream>

#include "stringencoder.h"

//Represents a block of text which may be common between the two files
struct Block {
    //The "run" of common text
    std::vector<int> run;
    //Indicates where in each file the block begins (end can be found using block.length)
    int oldloc;
    int newloc;
};

std::ostream& operator<<(std::ostream& os, const Block& bl);

//Compare blocks based on location in old file
bool compareOld(const Block* lhs, const Block* rhs);
//Compare blocks based on location in new file
bool compareNew(const Block* lhs, const Block* rhs);

std::vector<Block*> getCommonBlocks(int minsize, StringEncoder& se);
void extendBlocks(std::vector<Block*>& allblocks, StringEncoder& se);
void resolveIntersections(std::vector<Block*>& allblocks);
unsigned int hashVector(std::vector<int>& v);

#endif