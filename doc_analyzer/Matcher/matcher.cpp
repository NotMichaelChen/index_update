#include <vector>
#include <algorithm>

#include "Matcher/stringencoder.h"
#include "Matcher/block.h"
#include "Matcher/graph.h"
#include "Matcher/distancetable.h"
#include "postings.h"

using namespace std;

namespace Matcher {
    vector<Block*> getCommonBlocks(StringEncoder& se, int minblocksize, int maxblockcount, int selectionparameter) {
        //Find common blocks between the two files
        vector<Block*> commonblocks = getCommonBlocks(minblocksize, se);
        extendBlocks(commonblocks, se);
        resolveIntersections(commonblocks);
        
        //Create a graph of the common blocks
        BlockGraph G(commonblocks);
        vector<Block*> topsort = topologicalSort(G);
        
        //Get the optimal set of blocks to select
        DistanceTable disttable(maxblockcount, G, topsort);
        vector<Block*> finalpath = disttable.findOptimalPath(selectionparameter);
        
        return finalpath;
    }
    
    //TODO: refactor and decide if this should be one or two functions
    pair<vector<NonPositionalPosting>, vector<PositionalPosting>>
    getPostings(vector<Block*>& commonblocks, unsigned int doc_id, unsigned int fragID, StringEncoder& se) {
        //Which block to skip next
        int blockindex = 0;
        unordered_map<string, NonPositionalPosting> nppostingsmap;
        unordered_map<string, PositionalPosting> ppostingsmap;
        
        int index = 0;
        
        //Sort blocks based on oldindex first
        sort(commonblocks.begin(), commonblocks.end(), compareOld);
        while(index < se.getOldSize()) {
            if(index >= commonblocks[blockindex]->oldloc) {
                //This causes i to be located right after the common block
                index += commonblocks[blockindex]->run.size();
                ++blockindex;
            }
            else {
                //Edited sections in old file are considered "deleted", so decrement frequency
                
                string decodedword = se.decodeNum(*(se.getOldIter()+index));
                //Word already indexed
                if(nppostingsmap.find(decodedword) != nppostingsmap.end()) {
                    nppostingsmap.at(decodedword).freq -= 1;
                }
                else {
                    NonPositionalPosting newposting(decodedword, doc_id, -1);
                    nppostingsmap.insert(make_pair(decodedword, newposting));
                }
                ++index;
            }
        }
        
        index = 0;
        blockindex = 0;
        sort(commonblocks.begin(), commonblocks.end(), compareNew);
        while(index < se.getNewSize()) {
            if(index >= commonblocks[blockindex]->newloc) {
                //This causes i to be located right after the common block
                index += commonblocks[blockindex]->run.size();
                ++blockindex;
            }
            else {
                //Edited sections in new file are considered "inserted"
                string decodedword = se.decodeNum(*(se.getNewIter()+index));
                //Word already indexed in nonpositional map
                if(nppostingsmap.find(decodedword) != nppostingsmap.end()) {
                    nppostingsmap.at(decodedword).freq += 1;
                }
                else {
                    NonPositionalPosting newposting(decodedword, doc_id, 1);
                    nppostingsmap.insert(make_pair(decodedword, newposting));
                }
                //Always insert positional posting for a word
                PositionalPosting newposting(decodedword, doc_id, fragID, index);
                ppostingsmap.insert(make_pair(decodedword, newposting));
                ++index;
                ++fragID;
            }
        }
        
        vector<NonPositionalPosting> nppostingslist;
        nppostingslist.reserve(nppostingsmap.size());
        vector<PositionalPosting> ppostingslist;
        ppostingslist.reserve(ppostingsmap.size());
        
        for(auto kv : nppostingsmap) {
            nppostingslist.push_back(kv.second);
        }
        for(auto kv : ppostingsmap) {
            ppostingslist.push_back(kv.second);
        }
        
        return make_pair(nppostingslist, ppostingslist);
    }
}