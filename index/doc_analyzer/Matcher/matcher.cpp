#include "matcher.h"

#include <algorithm>

#include "distancetable.h"

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

    //Helper functions to make block traversal more clean
    bool incrementIndex(int beginloc, size_t blocklength, int& index, int& blockindex);

    pair<vector<ExternNPposting>, vector<ExternPposting>>
    getPostings(vector<shared_ptr<Block>>& commonblocks, unsigned int doc_id, unsigned int &fragID, StringEncoder& se) {
        //Which block to skip next
        int blockindex = 0;
        unordered_map<string, ExternNPposting> nppostingsmap;
        vector<ExternPposting> ppostingslist;

        //Sort blocks based on oldindex first
        sort(commonblocks.begin(), commonblocks.end(), compareOld);

        //Set index to either the end of the first block or to 0
        int index = 0;
        if(commonblocks.size() > 0 && index == commonblocks[0]->oldloc) {
            index = commonblocks[blockindex]->run.size();
        }
        
        while(index < se.getOldSize()) {
            string decodedword = se.decodeNum(*(se.getOldIter()+index));
            //Word not yet indexed
            if(nppostingsmap.find(decodedword) == nppostingsmap.end()) {
                ExternNPposting newposting(decodedword, doc_id, se.getNewCount(decodedword));
                nppostingsmap.insert(make_pair(decodedword, newposting));
            }

            //Condition prevents attempting to access an empty vector
            if(blockindex < commonblocks.size()) {
                incrementIndex(commonblocks[blockindex]->oldloc, commonblocks[blockindex]->run.size(), index, blockindex);
            }
            else
                ++index;
        }

        sort(commonblocks.begin(), commonblocks.end(), compareNew);

        index = 0;
        if(commonblocks.size() > 0 && index == commonblocks[0]->newloc) {
            index = commonblocks[blockindex]->run.size();
        }
        blockindex = 0;
        
        while(index < se.getNewSize()) {
            //Edited sections in new file are considered "inserted"
            string decodedword = se.decodeNum(*(se.getNewIter()+index));
            //Word not yet indexed in nonpositional map
            if(nppostingsmap.find(decodedword) == nppostingsmap.end()) {
                ExternNPposting newposting(decodedword, doc_id, se.getNewCount(decodedword));
                nppostingsmap.insert(make_pair(decodedword, newposting));
            }
            //Always insert positional posting for a word
            ExternPposting newposting(decodedword, doc_id, fragID, index);
            ppostingslist.push_back(newposting);

            if(blockindex < commonblocks.size()) {
                bool skip = incrementIndex(commonblocks[blockindex]->newloc, commonblocks[blockindex]->run.size(), index, blockindex);
                if(skip)
                    //When we skip a block of common text, we need a new fragID
                    ++fragID;
            }
            else
                ++index;
        }

        vector<ExternNPposting> nppostingslist;
        nppostingslist.reserve(nppostingsmap.size());

        for(auto kv : nppostingsmap) {
            nppostingslist.push_back(kv.second);
        }

        return make_pair(nppostingslist, ppostingslist);
    }

    //Helper functions to make block traversal more clean

    //Given an index and a block, either increment the index or skip the common block
    //Returns whether a skip was performed or not
    //To make the code reusable, pass only the block beginning and its length
    bool incrementIndex(int beginloc, size_t blocklength, int& index, int& blockindex) {
        ++index;
        
        if(index >= beginloc) {
            index += blocklength;
            ++blockindex;
            return true;
        }
        else return false;
    }
}
