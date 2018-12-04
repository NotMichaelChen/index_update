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
        landID = l;
        offset = o;
    }

    bool operator<(const Posting& rhs);

    unsigned int termID;
    unsigned int docID;
    unsigned int landID;
    unsigned int offset;
};

struct nPosting {
    nPosting() {}
    nPosting(unsigned int id, unsigned int d, unsigned int f) {
        termID = id;
        docID = d;
        frequency = f;
    }

    bool operator<(const nPosting& rhs);

    unsigned int termID;
    unsigned int docID;
    unsigned int frequency;
};