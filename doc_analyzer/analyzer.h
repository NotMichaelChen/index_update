#ifndef ANALYZER_H
#define ANALYZER_H

#include <fstream>
#include <vector>
#include <utility>

//General posting type used by the analyzer, to be further parsed by the index 
//Contains both position and nonpositional data and uses the actual word instead of a termid
//pos.size and freq may not exactly match due to deletions reducing freq without affecting pos
struct ProtoPosting {
    ProtoPosting(std::string id, unsigned int d, int fr = 0, unsigned int f = 0) {
        term = id;
        docID = d;
        freq = fr;
        fragID = f;
    }
    
    void insertPos(unsigned int newpos) {
        pos.push_back(newpos);
    }
    
    std::string term;
    unsigned int docID;
    int freq;
    unsigned int fragID;
    std::vector<unsigned int> pos;
};

struct NonPositionalPosting {
    NonPositionalPosting(std::string id, unsigned int d, int fr = 0) {
        term = id;
        docID = d;
        freq = fr;
    }
    
    std::string term;
    unsigned int docID;
    int freq;
};

struct PositionalPosting {
    PositionalPosting(std::string id, unsigned int d, int f, int p) {
        term = id;
        docID = d;
        fragID = f;
        pos = p;
    }
    
    std::string term;
    unsigned int docID;
    unsigned int fragID;
    int pos;
};

//Updates the index given a new page
void indexUpdate(std::string& url, std::ifstream& newpage);
//Generates new postings and translations from the new page
void makePosts(std::string& url, int doc_id, std::ifstream& oldpage, std::ifstream& newpage);
//Specifically generates postings given a vector of blocks
std::pair<std::vector<NonPositionalPosting>, std::vector<PositionalPosting>>
getPostings(std::vector<Block*>& commonblocks, int doc_id, int fragID, vector<int>& oldstream, vector<int>& newstream, StringEncoder& se)

#endif