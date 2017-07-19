#ifndef _INDEX_H_
#define _INDEX_H_

#include <vector>

class Index{
private:
	struct postingList{
		int termID;
		std::vector<unsigned int> postings;
		std::vector<unsigned int> pos;
	};

	std::vector<postingList> ind;

public:
	void update(int termID, unsigned int docID, unsigned int position);
	std::vector<postingList>::iterator find(int termID);
	void encode();
	void writeToDisk();
	void decode();
	void merge();
	void query();
	void display();
};

#endif