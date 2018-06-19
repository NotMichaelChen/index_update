#include "blockmatching.hpp"

#include <map>
#include <unordered_map>
#include <algorithm>

#include "util.hpp"

bool isOverlap(int oldbegin1, int newbegin1, int oldlen, int oldbegin2, int newbegin2, int newlen) {
    return
        oldbegin1 >= oldbegin2 &&
        oldbegin1 + oldlen <= oldbegin2 + newlen &&
        newbegin1 >= newbegin2 &&
        newbegin1 + oldlen <= newbegin2 + newlen;
}

std::vector<int> getExtended(std::vector<int>::const_iterator olditer, std::vector<int>::const_iterator newiter, StringEncoder& se) {
    auto begin = olditer;
    while(olditer != se.getOldEnd() && newiter != se.getNewEnd() && *olditer == *newiter) {
        olditer++;
        newiter++;
    }
    auto end = olditer;

    return std::vector<int>(begin, end);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<Block> getCommonBlocks(int minsize, StringEncoder& se) {
    std::vector<Block> commonblocks;
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
                bool isoverlap = false;
                //Potential overlap as long as this block's begin is after the other block's begin
                for(auto overlapchecker = commonblocks.rbegin(); overlapchecker != commonblocks.rend() &&
                    (newiter - se.getNewIter()) >= overlapchecker->newloc; overlapchecker++)
                {
                    if(isOverlap(matchedblock->second.oldloc, newiter - se.getNewIter(), minsize,
                        overlapchecker->oldloc, overlapchecker->newloc, overlapchecker->run.size()))
                    {
                        isoverlap = true;
                        break;
                    }
                }

                if(isoverlap)
                    continue;

                std::vector<int> extendedblock = getExtended(se.getOldIter() + matchedblock->second.oldloc, newiter, se);

                commonblocks.emplace_back(matchedblock->second.oldloc, newiter - se.getNewIter(), extendedblock);
            }
        }

        blockcheck.clear();
    }
    
    return commonblocks;
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