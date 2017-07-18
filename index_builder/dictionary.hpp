#ifndef _DICTIONARY_H_
#define _DICTIONARY_H_

#include <string>
#include <vector>
using namespace std;

class dictionary{
private:
	struct vocabulary{
		string term;
		int termID;
		long location; //starto location in the posting list
		int length; //length of the block in the posting list
	};

	vector<vocabulary> dict;
	int termID = 0;

public:
	vocabulary* find(string term);

	vocabulary *find(int id);

	void add(string term, int termID);

	int update(string voc);

	void update(string term, long loc, int len);

	void display();
	
};

#endif