#ifndef POSTING_H
#define POSTING_H

class Posting{
public:
	Posting ();

	Posting(unsigned int id, unsigned int d, unsigned int f = 0, unsigned int p = 0);

	friend bool operator< (Posting p1, Posting p2);

	friend bool operator> (Posting p1, Posting p2);

	friend bool operator== (Posting p1, Posting p2);

	unsigned int termID;
	unsigned int docID;
	unsigned int fragID;
	unsigned int pos;
};

#endif
