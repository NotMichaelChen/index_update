#include "analyzer.h"

#include <fstream>
#include <vector>
#include <unordered_map>
#include <utility>
#include <algorithm>

#include "Matcher/stringencoder.h"
#include "Matcher/block.h"
#include "Matcher/graph.h"
#include "Matcher/distancetable.h"
#include "Matcher/translate.h"

#define MIN_BLOCK_SIZE 10
#define MAX_BLOCK_COUNT 20

using namespace std;

/* TODO
 * max fragid can be found by looking at the translation list's size for the given document
 * decide whether to store max fragid in document store or use above trick
 * are the translation lists/document store global or passed in?
 * find out where posting structs should be defined
 * implement document store
 * implement block selection count calculation
 * time to start merging code with Fengyuan?
 * refactor everything
 */

void indexUpdate(string& url, ifstream& newpage) {
    //-fetch the previous version, and the did of the document, from a tuple store or database (TBD)

    //-call makePosts(URL, did, currentpage, previouspage), which generates and returns the new postings that you are creating by your matching algorithm (that is, non-positional and position postings) and the additional translation statements to be appended.

    //-now you can directly call Fengyuan's code to insert those posts(to be done later)

    //-and then append the translation commands to the right translation vector

    //-and store the currentpage instead of the previouspage in the tuple store.
}

void makePosts(string& url, unsigned int doc_id, ifstream& oldpage, ifstream& newpage) {
    //-check if there was a previous version, if not create postings with fragid = 0
    int fragID;
    
    if(!oldpage.is_open()) {
        fragID = 0;
    }

    //-else, run the graph based matching algorithm on the two versions
    //Encode both files as lists of numbers
    StringEncoder se;
    vector<int> oldstream = se.encodeFile(oldpage);
    vector<int> newstream = se.encodeFile(newpage);
    
    //Find common blocks between the two files
    vector<Block*> commonblocks = getCommonBlocks(MIN_BLOCK_SIZE, oldstream, newstream);
    extendBlocks(commonblocks, oldstream, newstream);
    resolveIntersections(commonblocks);
    
    //Create a graph of the common blocks
    BlockGraph G(commonblocks);
    vector<Block*> topsort = topologicalSort(G);
    
    //Get the optimal set of blocks to select
    DistanceTable disttable(MAX_BLOCK_COUNT, G, topsort);
    vector<pair<int, Block*>> bestlist = disttable.findAllBestPaths();
    vector<Block*> finalpath = disttable.tracePath(bestlist.back().second, bestlist.size());
    
    //Get the translation and posting list
    vector<Translation> translist = getTranslations(oldstream.size(), newstream.size(), finalpath);
    //TODO: get the maximum fragid and pass it in
    auto postingslist = getPostings(commonblocks, doc_id, 0, oldstream, newstream, se);
    //Number of fragIDs used is exactly proportional to the number of positional postings inserted
    fragID += postingslist.second.size();

    //-generate postings and translation statements, and return them. (Question: how do we know the previous largest fragid for this document, so we know what to use as the next fragid? Maybe store with did in the tuple store?)
}

//TODO: refactor and decide if this should be one or two functions
pair<vector<NonPositionalPosting>, vector<PositionalPosting>>
getPostings(vector<Block*>& commonblocks, unsigned int doc_id, unsigned int fragID, vector<int>& oldstream, vector<int>& newstream, StringEncoder& se) {
    //Which block to skip next
    int blockindex = 0;
    unordered_map<string, NonPositionalPosting> nppostingsmap;
    unordered_map<string, PositionalPosting> ppostingsmap;
    
    unsigned int index = 0;
    
    //Sort blocks based on oldindex first
    sort(commonblocks.begin(), commonblocks.end(), compareOld);
    while(index < oldstream.size()) {
        if(index >= commonblocks[blockindex]->oldloc) {
            //This causes i to be located right after the common block
            index += commonblocks[blockindex]->run.size();
            ++blockindex;
        }
        else {
            //Edited sections in old file are considered "deleted", so decrement frequency
            
            string decodedword = se.decodeNum(oldstream[index]);
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
    while(index < newstream.size()) {
        if(index >= commonblocks[blockindex]->newloc) {
            //This causes i to be located right after the common block
            index += commonblocks[blockindex]->run.size();
            ++blockindex;
        }
        else {
            //Edited sections in new file are considered "inserted"
            string decodedword = se.decodeNum(oldstream[index]);
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