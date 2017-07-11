#include <iostream>
#include <fstream>
#include <getopt.h>
#include <vector>

#include "stringencoder.h"
#include "block.h"
#include "graph.h"
#include "distancetable.h"

using namespace std;

/* Thing to do
 * 
 * refactor longestPath
 * maybe check for memory leaks using a debugger?
 * 
 */

int main(int argc, char **argv) {
    char k = -1;
    string oldfilename, newfilename;
    bool isold = false, isnew = false;
    
    int opt;
    while((opt = getopt(argc, argv, "k:o:n:")) != -1) {
        switch(opt) {
        case 'k':
            k = *optarg;
            break;
        case 'o':
            oldfilename = optarg;
            isold = true;
            break;
        case 'n':
            newfilename = optarg;
            isnew = true;
            break;
        }
    }
    
    int blocks = 0;
    if(isdigit(k)) {
        blocks = k - '0';
    }
    else {
        cout << "Error: Invalid number of blocks specified" << endl;
        exit(EXIT_FAILURE);
    }
    
    if(!isold) {
        cout << "Error: Old file not specified" << endl;
        exit(EXIT_FAILURE);
    }
    
    if(!isnew) {
        cout << "Error: New file not specified" << endl;
        exit(EXIT_FAILURE);
    }
    
    //Get filestreams of the two files
    ifstream oldfile(oldfilename);
    ifstream newfile(newfilename);
    
    //Encode both files as lists of numbers
    StringEncoder se;
    vector<int> oldstream = se.encodeFile(oldfile);
    vector<int> newstream = se.encodeFile(newfile);
    
    //Find common blocks between the two files
    vector<Block*> commonblocks = getCommonBlocks(10, oldstream, newstream);
    
    extendBlocks(commonblocks, oldstream, newstream);
    resolveIntersections(commonblocks);
    
    int counter = 0;
    for(Block* i : commonblocks){
        vector<string> block = se.decodeStream(i->run);
        for(string j : block)
            cout << j << " ";
        cout << endl << i->oldloc << "-" << i->newloc << "-" << i->run.size() << endl;
        counter++;
    }
    cout << counter << endl;
    
    BlockGraph G(commonblocks);
    vector<Block*> vertices = G.getAllVertices();
    for(Block* i : vertices) {
        cout << i->oldloc << "-" << i->newloc << endl;
        
        vector<Block*> edges = G.getAdjacencyList(i);
        for(Block* j : edges)
            cout << "\t" << j->oldloc << "-" << j->newloc << endl;
    }
    cout << endl;
    
    vector<Block*> topsort = topologicalSort(G);
    for(Block* b : topsort) {
        cout << b->oldloc << "-" << b->newloc << " ";
    }
    cout << endl;
    
    DistanceTable disttable(blocks, G, topsort);
    vector<pair<int, Block*>> bestlist = disttable.findAllBestPaths();
    for(size_t i = 0; i < bestlist.size(); ++i) {
        if(bestlist[i].first == 0) break;
        vector<Block*> path = disttable.tracePath(bestlist[i].second, i+1);
        cout << i << "\t" << bestlist[i].first << "\t";
        for(Block* j : path) {
            //cout << j;
            j->printloc();
            cout << " ";
        }
        cout << endl;
    }
    
    return 0;
}