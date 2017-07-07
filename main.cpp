#include <iostream>
#include <fstream>
#include <getopt.h>
#include <vector>

#include "stringencoder.h"
#include "block.h"
#include "graph.h"

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
    vector<Block*> commonblocks = getCommonBlocks(5, oldstream, newstream);
    
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
    
    map<Block*, vector<Block*>> G = makeGraph(commonblocks);
    for(auto iter = G.begin(); iter != G.end(); iter++) {
        if(iter->first != nullptr)
            cout << (iter->first)->oldloc << "-" << (iter->first)->newloc << endl;
        else
            cout << "S" << endl;
        
        for(auto edgeiter = iter->second.begin(); edgeiter != iter->second.end(); edgeiter++)
            cout << "\t" << (*edgeiter)->oldloc << "-" << (*edgeiter)->newloc << endl;
    }
    cout << endl;
    vector<Block*> topsort = topologicalSort(G);
    for(Block* b : topsort) {
        if(b == nullptr) continue;
        cout << b->oldloc << "-" << b->newloc << " ";
    }
    cout << endl;
    
    int length = longestPath(blocks, G, topsort);
    
    cout << length << endl;
    
    return 0;
}