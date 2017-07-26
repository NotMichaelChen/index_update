#include "analyzer.h"

#include <fstream>
#include <vector>
#include <unordered_map>

#include "stringencoder.h"
#include "block.h"
#include "graph.h"
#include "distancetable.h"
#include "translate.h"

#define MIN_BLOCK_SIZE 10

using namespace std;

void indexUpdate(string& url, ifstream& newpage) {
    //-fetch the previous version, and the did of the document, from a tuple store or database (TBD)

    //-call makePosts(URL, did, currentpage, previouspage), which generates and returns the new postings that you are creating by your matching algorithm (that is, non-positional and position postings) and the additional translation statements to be appended.

    //-now you can directly call Fengyuan's code to insert those posts(to be done later)

    //-and then append the translation commands to the right translation vector

    //-and store the currentpage instead of the previouspage in the tuple store.
}

void makePosts(string& url, int doc_id, ifstream& oldpage, ifstream& newpage) {
    //-check if there was a previous version, if not create postings with fragid = 0
    if(!newpage.is_open()) {
        
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
    DistanceTable disttable(blocks, G, topsort);
    vector<pair<int, Block*>> bestlist = disttable.findAllBestPaths();
    vector<Block*> finalpath = disttable.tracePath(bestlist.back().second, bestlist.size());
    
    //Get file lengths
    oldpage.seekg(0, ios::end);
    int oldlength = oldpage.tellg();
    newpage.seekg(0, ios::end);
    int newlength = newpage.tellg();
    vector<Translation> translist = getTranslations(oldlength, newlength, finalpath);

    //-generate postings and translation statements, and return them. (Question: how do we know the previous largest fragid for this document, so we know what to use as the next fragid? Maybe store with did in the tuple store?)
}

//TODO: Make fragid assignment correct
vector<ProtoPosting> getPostings(vector<Block*> commonblocks, int doc_id, vector<int>& oldstream, vector<int>& newstream, StringEncoder& se) {
    //Which block to skip next
    int blockindex = 0;
    unordered_map<string, ProtoPosting> postingsmap;
    
    size_t index = 0
    
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
            if(postingsmap.find(decodedword) != postingsmap.end()) {
                postingsmap[decodedword].freq -= 1;
            }
            else {
                ProtoPosting newposting(decodedword, doc_id, -1, 0);
                postingsmap[decodedword] = newposting;
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
            //Word already indexed
            if(postingsmap.find(decodedword) != postingsmap.end()) {
                postingsmap[decodedword].freq += 1;
                postingsmap[decodedword].insertPos(i);
            }
            else {
                ProtoPosting newposting(decodedword, doc_id, 1, 0);
                newposting.insertPos(i);
                postingsmap[decodedword] = newposting;
            }
            ++index;
        }
    }
    
    vector<ProtoPosting> postingslist;
    postingslist.reserve(postingsmap.size());
    
    for(auto kv : postingsmap) {
        postingslist.push_back(kv.second);
    }
    
    return postingslist;
}