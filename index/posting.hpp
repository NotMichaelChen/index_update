/**
 * Positional and non-positional posting structure.
 * These postings are of the format to be inserted into the index
 */

#ifndef POSTING_H
#define POSTING_H

struct Posting {
	Posting() {}
	Posting(unsigned int id, unsigned int d, unsigned int f = 0, unsigned int p = 0) {
        termID = id;
        docID = d;
        second = f; //fragmentID
        third = p; //position
    }

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
        second = f; //frequency
    }

	unsigned int termID;
	unsigned int docID;
	unsigned int second;
};

#endif
