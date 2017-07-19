#ifndef _INDEX_H_
#define _INDEX_H_

#include <vector>

class Index{
private:
	struct Posting{
		Posting(unsigned int id, unsigned int d, unsigned int f = 0, unsigned int p = 0){
			termID = id;
			docID = d;
			fragID = f;
			pos = p;
		}
		unsigned int termID;
		unsigned int docID;
		unsigned int fragID;
		unsigned int pos;
	};

	std::vector<Posting> ind;

public:
	void update(int termID, unsigned int docID, unsigned int position);
	std::vector<posting>::iterator find(int termID);
	void encode();
	void writeToDisk();
	void decode();
	void merge();
	void query();
	void display();
};

#endif