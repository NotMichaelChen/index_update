#include "blockmatching.hpp"

#include <map>
#include <algorithm>

#include "util.hpp"

namespace Matcher
{

std::vector<Block> getCommonBlocks(int minsize, StringEncoder& se) {
    std::vector<Block> commonblocks;
    //Impossible to have common blocks if one doc is smaller than the minimum block size
    if(se.getOldSize() < minsize || se.getNewSize() < minsize || minsize < 1)
        return commonblocks;
    
    //Stores a "candidate block", extracted from the old doc
    //Each candidate can be matched multiple times with blocks from the new doc 
    std::multimap<unsigned int, Block> potentialblocks;
    
    //Get all possible blocks in the old file
    std::vector<int> newblock;
    for(auto olditer = se.getOldIter(); olditer != se.getOldEnd() - (minsize-1); ++olditer) {
        newblock.insert(newblock.end(), olditer, olditer+minsize);
        unsigned int blockhash = Utility::hashVector(newblock);

        Block tempblock(olditer - se.getOldIter(), -1, move(newblock));
        potentialblocks.insert(std::make_pair(blockhash, tempblock));

        newblock.clear();
    }
    
    //Check each block in the new file for a match with a block in the old file
    std::vector<int> blockcheck;
    for(auto newiter = se.getNewIter(); newiter != se.getNewEnd() - (minsize-1); ++newiter) {
        blockcheck.insert(blockcheck.end(), newiter, newiter+minsize);
        unsigned int blockhash = Utility::hashVector(blockcheck);
        
        //Get all blocks that match the current block's hash
        auto blockmatchrange = potentialblocks.equal_range(blockhash);
        for(auto matchedblock = blockmatchrange.first; matchedblock != blockmatchrange.second; ++matchedblock) {
            if(blockcheck == matchedblock->second.run) {
                commonblocks.emplace_back(matchedblock->second.oldloc, newiter - se.getNewIter(), matchedblock->second.run);
            }
        }

        blockcheck.clear();
    }
    
    return commonblocks;
}

//Attempt to extend common blocks
void extendBlocks(std::vector<Block>& allblocks, StringEncoder& se) {
    //Sort based on old locations
    std::sort(allblocks.begin(), allblocks.end(), compareOld);
    //Create a set to store deleted locations
    std::unordered_set<size_t> deletedlocs;
    
    //Index of block to be extended
    size_t index = 0;
    while(index < allblocks.size()) {
        size_t oldblocklength = allblocks[index].run.size();
        //indexes are now pointing beyond the block
        auto olditer = se.getOldIter() + (allblocks[index].oldendloc()+1);
        auto newiter = se.getNewIter() + (allblocks[index].newendloc()+1);
        
        //Attempt to extend the block
        while(olditer != se.getOldEnd() && newiter != se.getNewEnd() && *olditer == *newiter) {
            //Append the new word
            allblocks[index].run.push_back(*olditer);
            ++olditer;
            ++newiter;
        }
        
        //if we successfully extended the block, check for possible overlaps
        if(allblocks[index].run.size() > oldblocklength) {
            auto overlapchecker = allblocks.begin() + index + 1;

            //Potential overlap as long as the other block's begin is before this block's end
            while(overlapchecker != allblocks.end() && overlapchecker->oldloc < (olditer - se.getOldIter())) {
                if(overlapchecker->oldloc >= allblocks[index].oldloc &&
                    overlapchecker->oldendloc() <= (olditer - se.getOldIter()) &&
                    overlapchecker->newloc >= allblocks[index].newloc &&
                    overlapchecker->newendloc() <= (newiter - se.getNewIter()))
                {
                    deletedlocs.insert((size_t)(overlapchecker - allblocks.begin()));
                }
                overlapchecker++;
            }
        }
        
        do {
            index++;
        } while(deletedlocs.find(index) != deletedlocs.end());
    }

    //Copy over valid blocks
    std::vector<Block> newblocks;
    for(size_t i = 0; i < allblocks.size(); i++) {
        if(deletedlocs.find(i) == deletedlocs.end()) {
            newblocks.push_back(allblocks[i]);
        }
    }
    allblocks.swap(newblocks);
}

//Resolve blocks that are intersecting
//Should be run after extendBlocks
void resolveIntersections(std::vector<Block>& allblocks) {
    //List of blocks to add to the main list later
    //NOTE: It is impossible for blocks generated from intersection resolution to
    //be a duplicate from an original block in allblocks.
    //Consider a b c d e, a b c f c d e. "a b" would be generated from intersection resolution,
    //but it is impossible for that block to be generated normally since it is possible to extend that block,
    //so the extended version would be added, which is a b c.
    std::vector<Block> addedblocks;
    //First, sort based on old locations
    std::sort(allblocks.begin(), allblocks.end(), compareOld);
    
    for(size_t i = 0; i < allblocks.size(); i++) {
        for(size_t j = i+1; j < allblocks.size(); j++) {
            //break if intersections are not possible anymore
            if(allblocks[j].oldloc > allblocks[i].oldendloc())
                break;
            
            //Intersection occurs if B.end > A.end > B.begin > A.begin
            //We know that B.begin > A.begin (iterating in sorted order)
            //We know that A.end > B.begin (is our breaking condition)
            //Thus we only need to check B.end > A.end
            if(allblocks[j].oldendloc() > allblocks[i].oldendloc()) {
                //calculate how much the current block needs to shrink by
                int shrunksize = allblocks[j].oldloc - allblocks[i].oldloc;
                //No point adding zero-length blocks
                if(shrunksize > 0) {
                    addedblocks.emplace_back(
                        allblocks[i].oldloc,
                        allblocks[i].newloc,
                        std::vector<int>(allblocks[i].run.begin(), allblocks[i].run.begin()+shrunksize)
                    );
                }
            }
        }
    }
    
    //Now sort based on new locations
    sort(allblocks.begin(), allblocks.end(), compareNew);
    
    for(size_t i = 0; i < allblocks.size(); i++) {
        for(size_t j = i+1; j < allblocks.size(); j++) {
            if(allblocks[j].newloc > allblocks[i].newendloc())
                break;
            
            if(allblocks[j].newendloc() > allblocks[i].newendloc()) {
                int shrunksize = allblocks[j].newloc - allblocks[i].newloc;
                if(shrunksize > 0) {
                    addedblocks.emplace_back(
                        allblocks[i].oldloc,
                        allblocks[i].newloc,
                        std::vector<int>(allblocks[i].run.begin(), allblocks[i].run.begin()+shrunksize)
                    );
                }
            }
        }
    }
    
    //remove duplicates
    std::sort(addedblocks.begin(), addedblocks.end(), compareOld);
    addedblocks.erase(std::unique(addedblocks.begin(), addedblocks.end()), addedblocks.end());
    //append the list of added blocks
    allblocks.insert(allblocks.end(), addedblocks.begin(), addedblocks.end());
}



}