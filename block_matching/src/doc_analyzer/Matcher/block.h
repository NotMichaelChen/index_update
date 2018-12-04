#ifndef BLOCK_H
#define BLOCK_H

#include <vector>
#include <memory>

#include "stringencoder.h"
#include "utility/util.hpp"

//Represents a block of text which may be common between the two files
struct Block {
    Block();
    Block(int o, int n, size_t l);
    
    int oldendloc();
    int newendloc();
    
    //Indicates where in each file the block begins
    int oldloc;
    int newloc;
    //Indicates the length of common text
    size_t len;
};


//Printing overload for blocks for debugging
std::ostream& operator<<(std::ostream& os, const Block& bl);

//Block operators
//Define outside struct since they work based on pointers
bool operator==(const std::shared_ptr<Block>& lhs, const std::shared_ptr<Block>& rhs);
//Compare blocks based on location in old file
bool compareOld(const std::shared_ptr<Block>& lhs, const std::shared_ptr<Block>& rhs);
//Compare blocks based on location in new file
bool compareNew(const std::shared_ptr<Block>& lhs, const std::shared_ptr<Block>& rhs);
bool compareSizeGreater(const std::shared_ptr<Block>& lhs, const std::shared_ptr<Block>& rhs);
//Compares blocks by old location, then new location, then by length
bool compareStrict(const std::shared_ptr<Block>& lhs, const std::shared_ptr<Block>& rhs);

#endif
