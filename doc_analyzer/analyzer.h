#ifndef ANALYZER_H
#define ANALYZER_H

#include <fstream>
#include <vector>

//General posting type used by the analyzer, to be further parsed by the index 
//Contains both position and nonpositional data and uses the actual word instead of a termid
//pos.size and freq may not exactly match due to deletions reducing freq without affecting pos
struct ProtoPosting {
    Posting(string id, unsigned int d, int fr = 0, unsigned int f = 0) {
        term = id;
        docID = d;
        freq = fr;
        fragID = f;
    }
    
    void insertPos(unsigned int newpos) {
        pos.push_back(newpos);
    }
    
    string term;
    unsigned int docID;
    int freq;
    unsigned int fragID;
    std::vector<unsigned int> pos;
};

//Updates the index given a new page
void indexUpdate(std::string& url, std::ifstream& newpage);
//Generates new postings and translations from the new page
void makePosts(std::string& url, int doc_id, std::ifstream& oldpage, std::ifstream& newpage);
//Specifically generates postings given a vector of blocks
std::vector<Posting> getPostings(std::vector<Block*> commonblocks, std::ifstream& oldpage, std::ifstream& newpage);

#endif