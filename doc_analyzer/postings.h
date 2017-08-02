#ifndef POSTINGS_H
#define POSTINGS_H

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

#endif