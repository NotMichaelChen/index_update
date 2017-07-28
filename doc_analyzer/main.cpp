#include <iostream>
#include <fstream>
#include <getopt.h>
#include <vector>

#include "Matcher/stringencoder.h"
#include "Matcher/block.h"
#include "Matcher/graph.h"
#include "Matcher/distancetable.h"
#include "Matcher/translate.h"

using namespace std;

int main(int argc, char **argv) {
    string k, oldfilename, newfilename;
    bool isold = false, isnew = false;
    
    int opt;
    while((opt = getopt(argc, argv, "k:o:n:")) != -1) {
        switch(opt) {
        case 'k':
            k = optarg;
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
    size_t checker;
    blocks = stoi(k, &checker, 10);
    if(checker != k.size()) {
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
        cout << *i << endl;
        counter++;
    }
    cout << counter << endl;
    
    BlockGraph G(commonblocks);
    vector<Block*> vertices = G.getAllVertices();
    for(Block* i : vertices) {
        cout << *i << endl;
        
        vector<Block*> edges = G.getAdjacencyList(i);
        for(Block* j : edges)
            cout << "\t" << *j << endl;
    }
    cout << endl;
    
    vector<Block*> topsort = topologicalSort(G);
    for(Block* b : topsort) {
        cout << *b;
    }
    cout << endl;
    
    DistanceTable disttable(blocks, G, topsort);
    vector<pair<int, Block*>> bestlist = disttable.findAllBestPaths();
    
    cout << "Steps\tWeight\tPath" << endl;
    for(size_t i = 0; i < bestlist.size(); ++i) {
        if(bestlist[i].first == 0) break;
        vector<Block*> path = disttable.tracePath(bestlist[i].second, i+1);
        cout << i+1 << "\t" << bestlist[i].first << "\t";
        for(Block* j : path) {
            //cout << j;
            cout << *j << " ";
        }
        cout << endl;
    }
    
    vector<Block*> finalpath = disttable.tracePath(bestlist.back().second, bestlist.size());
    
    vector<Translation> translist = getTranslations(oldstream.size(), newstream.size(), finalpath);
    
    for(Translation t : translist) {
        cout << t.loc << " " << t.oldlen << " " << t.newlen << endl;
    }
    
    return 0;
}