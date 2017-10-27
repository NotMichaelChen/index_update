#include "block.h"

#include <vector>
#include <algorithm>
#include <map>
#include <iostream>
#include <memory>

#include "stringencoder.h"

using namespace std;

namespace Matcher {
    ostream& operator<<(ostream& os, const Block& bl) {
        os << bl.oldloc << "-" << bl.newloc << "-" << bl.run.size();
        return os;
    }
    
    bool compareOld(const shared_ptr<Block>& lhs, const shared_ptr<Block>& rhs) {
        return lhs->oldloc < rhs->oldloc;
    }
    bool compareNew(const shared_ptr<Block>& lhs, const shared_ptr<Block>& rhs) {
        return lhs->newloc < rhs->newloc;
    }
    
    vector<shared_ptr<Block>> getCommonBlocks(int minsize, StringEncoder& se) {
        vector<shared_ptr<Block>> commonblocks;
        if(se.getOldSize() < minsize || se.getNewSize() < minsize)
            return commonblocks;
       
        //Stores a "candidate block", extracted from the old doc
        //Each candidate can be matched multiple times with blocks from the new doc 
        multimap<unsigned int, Block> potentialblocks;
       
        //Get all possible blocks in the old file
        for(auto olditer = se.getOldIter(); olditer != se.getOldEnd() - (minsize-1); ++olditer) {
            vector<int> newblock(olditer, olditer+minsize);
            unsigned int blockhash = hashVector(newblock);
            Block tempblock(olditer - se.getOldIter(), -1, newblock);
            potentialblocks.insert(make_pair(blockhash, tempblock));
        }
        
        //Check each block in the new file for a match with a block in the old file
        for(auto newiter = se.getNewIter(); newiter != se.getNewEnd() - (minsize-1); ++newiter) {
            vector<int> blockcheck(newiter, newiter+minsize);
            unsigned int blockhash = hashVector(blockcheck);
            
            //Get all blocks that match the current block's hash
            auto blockmatchrange = potentialblocks.equal_range(blockhash);
            for(auto matchedblock = blockmatchrange.first; matchedblock != blockmatchrange.second; ++matchedblock) {
                if(blockcheck == matchedblock->second.run) {
                    shared_ptr<Block> match = make_shared<Block>(
                        matchedblock->second.oldloc,
                        newiter - se.getNewIter(),
                        matchedblock->second.run
                    );
                    
                    commonblocks.push_back(match);
                }
            }
        }
        
        return commonblocks;
    }
    
    //Attempt to extend common blocks
    void extendBlocks(vector<shared_ptr<Block>>& allblocks, StringEncoder& se) {
        //Sort based on old locations
        sort(allblocks.begin(), allblocks.end(), compareOld);
        
        //Index of block to be extended
        size_t index = 0;
        while(index < allblocks.size()) {
            size_t oldblocklength = allblocks[index]->run.size();
            //indexes are now pointing beyond the block
            auto olditer = se.getOldIter() + (allblocks[index]->oldendloc()+1);
            auto newiter = se.getNewIter() + (allblocks[index]->newendloc()+1);
            
            //Attempt to extend the block
            while(olditer != se.getOldEnd() && newiter != se.getNewEnd() && *olditer == *newiter) {
                //Append the new word
                allblocks[index]->run.push_back(*olditer);
                ++olditer;
                ++newiter;
            }
            
            //if we successfully extended the block, check for possible overlaps
            if(allblocks[index]->run.size() > oldblocklength) {
                auto overlapchecker = allblocks.begin() + index + 1;
                //Potential overlap as long as the other block's begin is before this block's end
                while(overlapchecker != allblocks.end() && (*overlapchecker)->oldloc < (olditer - se.getOldIter())) {
                    if((*overlapchecker)->oldloc >= allblocks[index]->oldloc &&
                        (*overlapchecker)->oldendloc() <= (olditer - se.getOldIter()) &&
                        (*overlapchecker)->newloc >= allblocks[index]->newloc &&
                        (*overlapchecker)->newendloc() <= (newiter - se.getNewIter()))
                    {
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
    void resolveIntersections(vector<shared_ptr<Block>>& allblocks) {
        //List of blocks to add to the main list later
        vector<shared_ptr<Block>> addedblocks;
        //First, sort based on old locations
        sort(allblocks.begin(), allblocks.end(), compareOld);
        
        for(size_t i = 0; i < allblocks.size(); i++) {
            for(size_t j = i+1; j < allblocks.size(); j++) {
                //break if intersections are not possible anymore
                if(allblocks[j]->oldloc > allblocks[i]->oldendloc())
                    break;
                
                //Intersection occurs if B.end > A.end > B.begin > A.begin
                //We know that B.begin > A.begin (iterating in sorted order)
                //We know that A.end > B.begin (is our breaking condition)
                //Thus we only need to check B.end > A.end
                if(allblocks[j]->oldendloc() > allblocks[i]->oldendloc()) {
                    //calculate how much the current block needs to shrink by
                    int shrunksize = allblocks[j]->oldloc - allblocks[i]->oldloc;
                    shared_ptr<Block> shrunkblock = make_shared<Block>(
                        allblocks[i]->oldloc,
                        allblocks[i]->newloc,
                        vector<int>(allblocks[i]->run.begin(), allblocks[i]->run.begin()+shrunksize)
                    );
                    
                    addedblocks.push_back(shrunkblock);
                }
            }
        }
        
        //Now sort based on new locations
        sort(allblocks.begin(), allblocks.end(), compareNew);
        
        for(size_t i = 0; i < allblocks.size(); i++) {
            for(size_t j = i+1; j < allblocks.size(); j++) {
                if(allblocks[j]->newloc > allblocks[i]->newendloc())
                    break;
                
                if(allblocks[j]->newendloc() > allblocks[i]->newendloc()) {
                    int shrunksize = allblocks[j]->newloc - allblocks[i]->newloc;
                    shared_ptr<Block> shrunkblock = make_shared<Block>(
                        allblocks[i]->oldloc,
                        allblocks[i]->newloc,
                        vector<int>(allblocks[i]->run.begin(), allblocks[i]->run.begin()+shrunksize)
                    );
                    
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