#include "blockmatching.hpp"

#include <map>
#include <unordered_map>
#include <algorithm>
#include <iostream>

#include "utility/util.hpp"

//is 1 overlapped by 2
bool isOverlap(int oldbegin1, int newbegin1, int oldlen, int oldbegin2, int newbegin2, int newlen) {
    return
        oldbegin1 >= oldbegin2 &&
        oldbegin1 + oldlen <= oldbegin2 + newlen &&
        newbegin1 >= newbegin2 &&
        newbegin1 + oldlen <= newbegin2 + newlen;
}

std::vector<int> getExtended(std::vector<int>::const_iterator olditer, std::vector<int>::const_iterator newiter, StringEncoder& se) {
    auto oldend = se.getOldEnd();
    auto newend = se.getNewEnd();

    auto begin = olditer;
    while(olditer != oldend && newiter != newend && *olditer == *newiter) {
        olditer++;
        newiter++;
    }
    auto end = olditer;

    return std::vector<int>(begin, end);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<std::shared_ptr<Block>> getCommonBlocks(int minsize, StringEncoder& se) {
    std::vector<std::shared_ptr<Block>> commonblocks;
    //Impossible to have common blocks if one doc is smaller than the minimum block size
    if(se.getOldSize() < minsize || se.getNewSize() < minsize || minsize < 1)
        return commonblocks;
    
    //Stores a "candidate block", extracted from the old doc
    //Each candidate can be matched multiple times with blocks from the new doc 
    std::unordered_multimap<unsigned int, Block> potentialblocks;
    
    //Get all possible blocks in the old file
    std::vector<int> newblock;
    for(auto olditer = se.getOldIter(); olditer != se.getOldEnd() - (minsize-1); ++olditer) {
        newblock.insert(newblock.end(), olditer, olditer+minsize);
        unsigned int blockhash = Utility::hashVector(newblock);

        Block tempblock(olditer - se.getOldIter(), -1, move(newblock));
        potentialblocks.insert(std::make_pair(blockhash, tempblock));

        newblock.clear();
    }

    auto newbeginiter = se.getNewIter();
    
    //Check each block in the new file for a match with a block in the old file
    std::vector<int> blockcheck;
    for(auto newiter = se.getNewIter(); newiter != se.getNewEnd() - (minsize-1); ++newiter) {
        blockcheck.insert(blockcheck.end(), newiter, newiter+minsize);
        unsigned int blockhash = Utility::hashVector(blockcheck);
        
        //Get all blocks that match the current block's hash
        auto blockmatchrange = potentialblocks.equal_range(blockhash);
        
        for(auto matchedblock = blockmatchrange.first; matchedblock != blockmatchrange.second; ++matchedblock) {
            //Double-check equality
            if(blockcheck == matchedblock->second.run) {
                commonblocks.push_back(std::make_shared<Block>(matchedblock->second.oldloc, newiter - newbeginiter, matchedblock->second.run));
            }
        }

        blockcheck.clear();
    }

    return commonblocks;
}

void extendBlocks(std::vector<std::shared_ptr<Block>>& commonblocks, StringEncoder& se) {
    if(commonblocks.size() <= 1)
        return;

    //Sort based on old locations
    std::sort(commonblocks.begin(), commonblocks.end(), compareStrict);
    //Create a vector of chars to store deleted locations
    //Can't use bools since vector<bool> is special
    std::vector<char> isdeleted(commonblocks.size());

    //Extend every block
    for(auto iter = commonblocks.begin(); iter != commonblocks.end(); iter++) {
        auto olditer = se.getOldIter() + (*iter)->oldloc;
        auto newiter = se.getNewIter() + (*iter)->newloc;
        (*iter)->run = getExtended(olditer, newiter, se);
    }

    //Eliminate overlaps
    size_t index = 0;
    while(index < commonblocks.size()) {
            
            //Potential overlap as long as the other block's begin is before this block's end
            int oldendloc = commonblocks[index]->oldendloc();
            for(auto overlapchecker = commonblocks.begin() + index + 1; overlapchecker != commonblocks.end() && (*overlapchecker)->oldloc <= oldendloc; overlapchecker++) {
                if(isdeleted[overlapchecker - commonblocks.begin()])
                    continue;
                
                if(isOverlap((*overlapchecker)->oldloc, (*overlapchecker)->newloc, (*overlapchecker)->run.size(),
                    commonblocks[index]->oldloc, commonblocks[index]->newloc, commonblocks[index]->run.size()))
                {
                    isdeleted[overlapchecker - commonblocks.begin()] = 1;
                }
            }
        
        do {
            index++;
        } while(index < commonblocks.size() && isdeleted[index]);
    }

    //Copy over valid blocks
    std::vector<std::shared_ptr<Block>> newblocks;
    for(size_t i = 0; i < commonblocks.size(); i++) {
        if(!isdeleted[i])
            newblocks.push_back(commonblocks[i]);
    }

    commonblocks.swap(newblocks);
}

//Resolve blocks that are intersecting
//Should be run after extendBlocks
void resolveIntersections(std::vector<std::shared_ptr<Block>>& allblocks) {
    //List of blocks to add to the main list later
    //NOTE: It is impossible for blocks generated from intersection resolution to
    //be a duplicate from an original block in allblocks.
    //Consider a b c d e, a b c f c d e. "a b" would be generated from intersection resolution,
    //but it is impossible for that block to be generated normally since it is possible to extend that block,
    //so the extended version would be added, which is a b c.
    std::vector<std::shared_ptr<Block>> addedblocks;
    //First, sort based on old locations
    std::sort(allblocks.begin(), allblocks.end(), compareOld);
    
    for(size_t i = 0; i < allblocks.size(); i++) {
        for(size_t j = i+1; j < allblocks.size(); j++) {
            //break if intersections are not possible anymore
            if(allblocks[j]->oldloc > allblocks[i]->oldendloc())
                break;
            
            //Intersection occurs if B.end > A.end >= B.begin > A.begin
            //We know that B.begin > A.begin (iterating in sorted order)
            //We know that A.end >= B.begin (is our breaking condition)
            //Thus we only need to check B.end > A.end
            if(allblocks[j]->oldendloc() > allblocks[i]->oldendloc()) {
                //calculate how much the current block needs to shrink by
                int shrunksize = allblocks[j]->oldloc - allblocks[i]->oldloc;
                //No point adding zero-length blocks
                if(shrunksize > 0) {
                    addedblocks.push_back(std::make_shared<Block>(
                        allblocks[i]->oldloc,
                        allblocks[i]->newloc,
                        std::vector<int>(allblocks[i]->run.begin(), allblocks[i]->run.begin()+shrunksize)
                    ));
                }
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
                if(shrunksize > 0) {
                    addedblocks.push_back(std::make_shared<Block>(
                        allblocks[i]->oldloc,
                        allblocks[i]->newloc,
                        std::vector<int>(allblocks[i]->run.begin(), allblocks[i]->run.begin()+shrunksize)
                    ));
                }
            }
        }
    }

    //remove duplicates
    std::sort(addedblocks.begin(), addedblocks.end(), compareStrict);
    addedblocks.erase(std::unique(addedblocks.begin(), addedblocks.end()), addedblocks.end());
    //append the list of added blocks
    allblocks.insert(allblocks.end(), addedblocks.begin(), addedblocks.end());
}