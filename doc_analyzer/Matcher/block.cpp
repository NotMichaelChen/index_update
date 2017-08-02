#include "block.h"

#include <vector>
#include <algorithm>
#include <map>
#include <iostream>

#include "stringencoder.h"

using namespace std;

namespace Matcher {
    ostream& operator<<(ostream& os, const Block& bl) {
        os << bl.oldloc << "-" << bl.newloc << "-" << bl.run.size();
        return os;
    }
    
    bool compareOld(const Block* lhs, const Block* rhs) {
        return lhs->oldloc < rhs->oldloc;
    }
    bool compareNew(const Block* lhs, const Block* rhs) {
        return lhs->newloc < rhs->newloc;
    }
    
    //Gets all common blocks between the two files of length minsize
    vector<Block*> getCommonBlocks(int minsize, StringEncoder& se) {
        vector<Block*> commonblocks;
        
        //Represents all possible blocks from the old file
        //Last element of vector is oldloc
        //<hash, block>
        //Hash is computed without oldloc in vector
        multimap<unsigned int, vector<int>> potentialblocks;
        
        //Get all possible blocks in the old file
        //no need to subtract 1 from se.getOldEnd, since it points past the end
        for(auto olditer = se.getOldIter(); olditer != se.getOldEnd() - minsize; ++olditer) {
            vector<int> newblock(olditer, olditer+minsize);
            //hash the block before inserting oldloc
            unsigned int blockhash = hashVector(newblock);
            //calculation is equivalent to getting the index of olditer
            newblock.push_back(olditer - se.getOldIter());
            potentialblocks.insert(make_pair(blockhash, newblock));
        }
        
        //Check each block in the new file for a match with a block in the old file
        for(auto newiter = se.getNewIter(); newiter != se.getNewEnd() - minsize; ++newiter) {
            vector<int> blockcheck(newiter, newiter+minsize);
            unsigned int blockhash = hashVector(blockcheck);
            
            //Get all blocks that match the current block's hash
            auto blockmatchrange = potentialblocks.equal_range(blockhash);
            for(auto iter = blockmatchrange.first; iter != blockmatchrange.second; ++iter) {
                //Remove oldloc before confirming equality
                int oldloc = (*iter).second.back();
                (*iter).second.pop_back();
                if(blockcheck == (*iter).second) {
                    Block* newblock = new Block;
                    newblock->run = blockcheck;
                    newblock->oldloc = oldloc;
                    newblock->newloc = newiter - se.getNewIter();
                    
                    commonblocks.push_back(newblock);
                }
                //Put oldloc back for later use
                (*iter).second.push_back(oldloc);
            }
        }
        
        return commonblocks;
    }
    
    //Attempt to extend common blocks
    void extendBlocks(vector<Block*>& allblocks, StringEncoder& se) {
        //Sort based on old locations
        sort(allblocks.begin(), allblocks.end(), compareOld);
        
        //Index of block to be extended
        size_t index = 0;
        while(index < allblocks.size()) {
            size_t blocklength = allblocks[index]->run.size();
            //indexes are now pointing beyond the block
            int oldfileindex = allblocks[index]->oldloc + blocklength;
            int newfileindex = allblocks[index]->newloc + blocklength;
            auto olditer = se.getOldIter()+oldfileindex;
            auto newiter = se.getNewIter()+newfileindex;
            
            //Attempt to extend the block
            while(olditer != se.getOldEnd() && newiter != se.getNewEnd() && *olditer == *newiter) {
                //Append the new word
                allblocks[index]->run.push_back(*olditer);
                ++olditer;
                ++newiter;
                ++oldfileindex;
                ++newfileindex;
            }
            
            //if we successfully extended the block, check for possible overlaps
            if(allblocks[index]->run.size() > blocklength) {
                auto overlapchecker = allblocks.begin() + index + 1;
                //Potential overlap as long as the other block's begin is before this block's end
                while(overlapchecker != allblocks.end() && (*overlapchecker)->oldloc < oldfileindex) {
                    int overlaplength = (*overlapchecker)->run.size()-1;
                    
                    if((*overlapchecker)->oldloc >= allblocks[index]->oldloc &&
                        (*overlapchecker)->oldloc+overlaplength <= (olditer - se.getOldIter()) &&
                        (*overlapchecker)->newloc >= allblocks[index]->newloc &&
                        (*overlapchecker)->newloc+overlaplength <= (newiter - se.getNewIter()))
                    {
                        delete *overlapchecker;
                        overlapchecker = allblocks.erase(overlapchecker);
                    }
                    else
                        overlapchecker++;
                }
            }
            
            index++;
        }
    }
    
    //Resolve blocks that are intersecting
    //Should be run after extendBlocks
    void resolveIntersections(vector<Block*>& allblocks) {
        //List of blocks to add to the main list later
        vector<Block*> addedblocks;
        //First, sort based on old locations
        sort(allblocks.begin(), allblocks.end(), compareOld);
        
        for(size_t i = 0; i < allblocks.size(); i++) {
            for(size_t j = i+1; j < allblocks.size(); j++) {
                //break if intersections are not possible anymore
                if(allblocks[j]->oldloc > allblocks[i]->oldloc + allblocks[i]->run.size()-1)
                    break;
                
                //Intersection occurs if B.end > A.end > B.begin > A.begin
                //We know that B.begin > A.begin (iterating in sorted order)
                //We know that A.end > B.begin (is our breaking condition)
                //Thus we only need to check B.end > A.end
                if(allblocks[j]->oldloc + allblocks[j]->run.size()-1 > allblocks[i]->oldloc + allblocks[i]->run.size()-1) {
                    //calculate how much the current block needs to shrink by
                    int shrunksize = allblocks[j]->oldloc - allblocks[i]->oldloc;
                    Block* shrunkblock = new Block;
                    shrunkblock->run = vector<int>(allblocks[i]->run.begin(), allblocks[i]->run.begin()+shrunksize);
                    shrunkblock->oldloc = allblocks[i]->oldloc;
                    shrunkblock->newloc = allblocks[i]->newloc;
                    addedblocks.push_back(shrunkblock);
                }
            }
        }
        
        //Now sort based on new locations
        sort(allblocks.begin(), allblocks.end(), compareNew);
        
        for(size_t i = 0; i < allblocks.size(); i++) {
            for(size_t j = i+1; j < allblocks.size(); j++) {
                if(allblocks[j]->newloc > allblocks[i]->newloc + allblocks[i]->run.size()-1)
                    break;
                
                if(allblocks[j]->newloc + allblocks[j]->run.size()-1 > allblocks[i]->newloc + allblocks[i]->run.size()-1) {
                    int shrunksize = allblocks[j]->newloc - allblocks[i]->newloc;
                    Block* shrunkblock = new Block;
                    shrunkblock->run = vector<int>(allblocks[i]->run.begin(), allblocks[i]->run.begin()+shrunksize);
                    shrunkblock->oldloc = allblocks[i]->oldloc;
                    shrunkblock->newloc = allblocks[i]->newloc;
                    addedblocks.push_back(shrunkblock);
                }
            }
        }
        
        //append the list of added blocks
        allblocks.insert(allblocks.end(), addedblocks.begin(), addedblocks.end());
    }
    
    //Credit to https://stackoverflow.com/a/3404820
    //Does not need to be optimal; we will confirm equality later anyways
    //Must be unsigned since overflow is undefined for signed ints
    unsigned int hashVector(vector<int>& v) {
        unsigned int hc = v.size();
        for(size_t i = 0; i < v.size(); i++)
        {
            hc = hc*314159 + v[i];
        }
        return hc;
    }
}