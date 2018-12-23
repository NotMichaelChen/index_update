/**
 * Positional and non-positional posting structure.
 * These postings are of the format to be inserted into the index
 */

#pragma once

struct Posting {
    Posting() {}
    Posting(unsigned int id, unsigned int d, unsigned int l = 0, unsigned int o = 0) {
        termID = id;
        docID = d;
        second = l;
        third = o;
    }

    bool operator<(const Posting& rhs);

    unsigned int termID;
    unsigned int docID;
    unsigned int second;
    unsigned int third;
};

struct nPosting {
    nPosting() {}
    nPosting(unsigned int id, unsigned int d, unsigned int f) {
        termID = id;
        docID = d;
        second = f;
    }

    bool operator<(const nPosting& rhs);

    unsigned int termID;
    unsigned int docID;
    unsigned int second;
    unsigned int third; // TODO: figure out way to remove
};