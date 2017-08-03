#ifndef BLOCK_H
#define BLOCK_H

#include <vector>
#include <iostream>

#include "stringencoder.h"

//Extracts all potential common blocks between file versions that may be selected by the graph algorithm

namespace Matcher {
    //Represents a block of text which may be common between the two files
    struct Block {
        Block(int o, int n, std::vector<int> b) : run(b), oldloc(o), newloc(n)  {}
        
        int oldendloc() { return oldloc + run.size() - 1; }
        int newendloc() { return newloc + run.size() - 1; }
            
        //The "run" of common text
        std::vector<int> run;
        //Indicates where in each file the block begins
        int oldloc;
        int newloc;
    };
    
    //Printing overload for blocks for debugging
    std::ostream& operator<<(std::ostream& os, const Block& bl);
    
    //Compare blocks based on location in old file
    bool compareOld(const Block* lhs, const Block* rhs);
    //Compare blocks based on location in new file
    bool compareNew(const Block* lhs, const Block* rhs);
    
    //Gets all possible common blocks of text of size minsize
    //Blocks can be overlapping; this is fixed with the other two functions
    std::vector<Block*> getCommonBlocks(int minsize, StringEncoder& se);
    //Extends common blocks, and removes blocks that are overlapped by the extended block
    void extendBlocks(std::vector<Block*>& allblocks, StringEncoder& se);
    //Resolves blocks that may be only partially overlapping
    //This is done by adding extra blocks that represent the 
    void resolveIntersections(std::vector<Block*>& allblocks);
    //Hashes a vector of ints
    //Not guaranteed to be optimal
    unsigned int hashVector(std::vector<int>& v);
}

#endif