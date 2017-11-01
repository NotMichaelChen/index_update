#include "matcher.h"

#include <vector>
#include <algorithm>
#include <memory>

#include "Matcher/stringencoder.h"
#include "Matcher/block.h"
#include "Matcher/graph.h"
#include "Matcher/distancetable.h"
#include "externalpostings.h"

using namespace std;

namespace Matcher {
    vector<shared_ptr<Block>> getOptimalBlocks(StringEncoder& se, int minblocksize, int maxblockcount, int selectionparameter) {
        //Find common blocks between the two files
        vector<shared_ptr<Block>> commonblocks = getCommonBlocks(minblocksize, se);
        extendBlocks(commonblocks, se);
        resolveIntersections(commonblocks);
        
        //Create a graph of the common blocks
        BlockGraph G(commonblocks);
        vector<shared_ptr<Block>> topsort = topologicalSort(G);
        
        //Get the optimal set of blocks to select
        DistanceTable disttable(maxblockcount, G, topsort);
        vector<shared_ptr<Block>> finalpath = disttable.findOptimalPath(selectionparameter);
        
        return finalpath;
    }
    
    //TODO: refactor and decide if this should be one or two functions
    pair<vector<ExternNPposting>, vector<ExternPposting>>
    getPostings(vector<shared_ptr<Block>>& commonblocks, unsigned int doc_id, unsigned int fragID, StringEncoder& se) {
        //Which block to skip next
        int blockindex = 0;
        unordered_map<string, ExternNPposting> nppostingsmap;
        vector<ExternPposting> ppostingslist;
        
        int index = 0;
        
        //Sort blocks based on oldindex first
        sort(commonblocks.begin(), commonblocks.end(), compareOld);
        while(index < se.getOldSize()) {
            if(blockindex < commonblocks.size() && index >= commonblocks[blockindex]->oldloc) {
                //This causes i to be located right after the common block
                index += commonblocks[blockindex]->run.size();
                ++blockindex;
            }
            else {
                //Edited sections in old file are considered "deleted", so decrement frequency
                
                string decodedword = se.decodeNum(*(se.getOldIter()+index));
                //Word not indexed yet indexed
                if(nppostingsmap.find(decodedword) == nppostingsmap.end()) {
                    ExternNPposting newposting(decodedword, doc_id, se.getNewCount(decodedword));
                    nppostingsmap.insert(make_pair(decodedword, newposting));
                }
                ++index;
            }
        }
        
        index = 0;
        blockindex = 0;
        sort(commonblocks.begin(), commonblocks.end(), compareNew);
        while(index < se.getNewSize()) {
            if(blockindex < commonblocks.size() && index >= commonblocks[blockindex]->newloc) {
                //This causes i to be located right after the common block
                index += commonblocks[blockindex]->run.size();
                ++blockindex;
                //When we skip a block of common text, we need a new fragID
                ++fragID;
            }
            else {
                //Edited sections in new file are considered "inserted"
                string decodedword = se.decodeNum(*(se.getNewIter()+index));
                //Word not yet indexed in nonpositional map
                if(nppostingsmap.find(decodedword) != nppostingsmap.end()) {
                    ExternNPposting newposting(decodedword, doc_id, se.getNewCount(decodedword));
                    nppostingsmap.insert(make_pair(decodedword, newposting));
                }
                //Always insert positional posting for a word
                ExternPposting newposting(decodedword, doc_id, fragID, index);
                ppostingslist.push_back(newposting);
                ++index;
            }
        }
        
        vector<ExternNPposting> nppostingslist;
        nppostingslist.reserve(nppostingsmap.size());
        
        for(auto kv : nppostingsmap) {
            nppostingslist.push_back(kv.second);
        }
        
        return make_pair(nppostingslist, ppostingslist);
    }
}
