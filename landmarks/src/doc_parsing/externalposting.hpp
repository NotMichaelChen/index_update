#pragma once

#include <string>

//These structs define the format of the posting returned by the matcher to the index
//The index uses a different form of postings, and must convert these postings
//when receiving them from the matcher

struct ExternNPposting {
    ExternNPposting(std::string id, unsigned int d, int fr = 0) {
        term = id;
        docID = d;
        freq = fr;
    }

    std::string term;
    unsigned int docID;
    int freq;
};

struct ExternPposting {
    ExternPposting(std::string id, unsigned int d, unsigned int l, unsigned int o) {
        term = id;
        docID = d;
        landID = l;
        offset = o;
    }

    std::string term;
    unsigned int docID;
    unsigned int landID;
    unsigned int offset;
};