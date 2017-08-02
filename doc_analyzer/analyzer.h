#ifndef ANALYZER_H
#define ANALYZER_H

#include <vector>
#include <utility>

#include "Matcher/stringencoder.h"
#include "Matcher/block.h"

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
    PositionalPosting(std::string id, unsigned int d, unsigned int f, unsigned int p) {
        term = id;
        docID = d;
        fragID = f;
        pos = p;
    }
    
    std::string term;
    unsigned int docID;
    unsigned int fragID;
    unsigned int pos;
};

//Updates the index given a new page
void indexUpdate(std::string& url, std::string& newpage);
//Generates new postings and translations from the new page
void makePosts(std::string& url, unsigned int doc_id, std::string& oldpage, std::string& newpage);
//Specifically generates postings given a vector of blocks
//fragID refers to the next ID to use
std::pair<std::vector<NonPositionalPosting>, std::vector<PositionalPosting>>
    getPostings(std::vector<Block*>& commonblocks, unsigned int doc_id, unsigned int fragID, StringEncoder& se);

#endif