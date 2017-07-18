#ifndef _INDEX_H_
#define _INDEX_H_

#include <vector>
using namespace std;
class index{
private:
	struct postingList{
		int termID;
		vector<unsigned int> postings;
		vector<unsigned int> pos;
	};

	vector<postingList> ind;

public:
	void update(int termID, unsigned int docID, unsigned int position);
	vector<postingList>::iterator find(int termID){
	void encode();
	void writeToDisk();
	void decode();
	void merge();
	void query();
	void display();
};

#endif