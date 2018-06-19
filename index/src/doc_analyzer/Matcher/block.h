#ifndef BLOCK_H
#define BLOCK_H

#include <vector>

#include "stringencoder.h"
#include "util.hpp"

//Represents a block of text which may be common between the two files
struct Block {
    Block();
    Block(int o, int n, std::vector<int> b);
    
    int oldendloc();
    int newendloc();
    bool isValid();
        
    //The "run" of common text
    std::vector<int> run;
    //Indicates where in each file the block begins
    int oldloc;
    int newloc;
};


//Printing overload for blocks for debugging
std::ostream& operator<<(std::ostream& os, const Block& bl);

//Block operators
//Define outside struct since they work based on pointers
bool operator==(const Block& lhs, const Block& rhs);
//Compare blocks based on location in old file
bool compareOld(const Block& lhs, const Block& rhs);
//Compare blocks based on location in new file
bool compareNew(const Block& lhs, const Block& rhs);
bool compareSizeGreater(const Block& lhs, const Block& rhs);


namespace std {
    //Allows for hashing of Block
    template <>
    struct hash<Block>
    {
        //http://en.cppreference.com/w/cpp/utility/hash
        std::size_t operator()(const Block& b) const
        {

        return ((hash<int>()(b.oldloc)
                ^ (hash<int>()(b.newloc) << 1)) >> 1)
                ^ (Utility::hashVector(b.run) << 1);
        }
    };
}

#endif
