#include <iostream>
#include <fstream>
#include <getopt.h>
#include <vector>
#include <algorithm>

#include "Matcher/matcher.h"
#include "Matcher/stringencoder.h"
#include "Matcher/block.h"
#include "Matcher/graph.h"
#include "Matcher/distancetable.h"
#include "Matcher/translate.h"
#include "Structures/documentstore.h"
#include "Structures/translationtable.h"

using namespace std;

int matchertest(int argc, char **argv);
int doctest();
int transtest();

int main(int argc, char **argv) {
    //return doctest();
    return transtest();
    //return matchertest(argc, argv);
    return 0;
}

int transtest() {
    Structures::TranslationTable table;
    vector<Matcher::Translation> trans;
    trans.push_back(Matcher::Translation(2, 4, 6));
    trans.push_back(Matcher::Translation(3, 5, 7));
    trans.push_back(Matcher::Translation(4, 6, 9));
    
    table.insert(trans, 0);
    reverse(trans.begin(), trans.end());
    table.insert(trans, 1);
    int loc = table.apply(0, 0, 5);
    
    cout << loc << endl;
    
    return 0;
}

int doctest() {
    Structures::DocumentStore store;
    
    store.insertDocument("www.abc.com", "This is a document", 123, "23-01-3203");
    store.insertDocument("www.def.com", "Different document", 2, "12-03-2323");
    Structures::DocumentTuple doc = store.getDocument("www.abc.com");
    cout << doc.docID << " " << doc.doc << " " << doc.maxfragID << " " << doc.timestamp << endl;
    doc = store.getDocument("www.def.com");
    cout << doc.docID << " " << doc.doc << " " << doc.maxfragID << " " << doc.timestamp << endl;
    
    return 0;
}

int matchertest(int argc, char **argv) {
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
    ifstream oldstream(oldfilename);
    ifstream newstream(newfilename);
    
    //https://www.reddit.com/r/learnprogramming/comments/3qotqr/how_can_i_read_an_entire_text_file_into_a_string/cwh8m4d/
    string oldfile{ istreambuf_iterator<char>(oldstream), istreambuf_iterator<char>() };
    string newfile{ istreambuf_iterator<char>(newstream), istreambuf_iterator<char>() };
    
    //Encode both files as lists of numbers
    Matcher::StringEncoder se(oldfile, newfile);
    
    //Find common blocks between the two files
    vector<Matcher::Block*> commonblocks = Matcher::getCommonBlocks(10, se);
    
    Matcher::extendBlocks(commonblocks, se);
    Matcher::resolveIntersections(commonblocks);
    
    int counter = 0;
    for(Matcher::Block* i : commonblocks){
        cout << *i << endl;
        counter++;
    }
    cout << counter << endl;
    
    Matcher::BlockGraph G(commonblocks);
    vector<Matcher::Block*> vertices = G.getAllVertices();
    for(Matcher::Block* i : vertices) {
        cout << *i << endl;
        
        vector<Matcher::Block*> edges = G.getAdjacencyList(i);
        for(Matcher::Block* j : edges)
            cout << "\t" << *j << endl;
    }
    cout << endl;
    
    vector<Matcher::Block*> topsort = Matcher::topologicalSort(G);
    for(Matcher::Block* b : topsort) {
        cout << *b << " ";
    }
    cout << endl;
    
    Matcher::DistanceTable disttable(blocks, G, topsort);
    vector<Matcher::DistanceTable::TableEntry> bestlist = disttable.findAllBestPaths();
    
    cout << "Steps\tWeight\tPath" << endl;
    for(size_t i = 0; i < bestlist.size(); ++i) {
        if(bestlist[i].current == nullptr) break;
        
        vector<Matcher::Block*> path = disttable.tracePath(bestlist[i]);
        cout << bestlist[i].steps << "\t" << bestlist[i].distance << "\t";
        for(Matcher::Block* j : path) {
            //cout << j;
            cout << *j << " ";
        }
        cout << endl;
    }
    
    vector<Matcher::Block*> finalpath = disttable.findOptimalPath(5);
    
    for(Matcher::Block* b : finalpath)
        cout << *b << endl;
    
    vector<Matcher::Translation> translist = Matcher::getTranslations(se.getOldSize(), se.getNewSize(), finalpath);
    
    for(Matcher::Translation t : translist) {
        cout << t.loc << " " << t.oldlen << " " << t.newlen << endl;
    }
    
    auto postings = Matcher::getPostings(finalpath, 0, 0, se);
    
    for(auto p : postings.first) {
        cout << p.term << " " << p.docID << " " << p.freq << endl;
    }
    
    for(auto p : postings.second) {
        cout << p.term << " " << p.docID << " " << p.fragID << " " << p.pos <<  endl;
    }
        
    
    return 0;
}