#ifndef _DICTIONARY_H_
#define _DICTIONARY_H_

#include <string>
#include <vector>

class Dictionary{
private:
	struct vocabulary{
		std::string term;
		int termID;
		long location; //starto location in the posting list
		int length; //length of the block in the posting list
	};

	std::vector<vocabulary> dict;
	int termID = 0;

public:
	vocabulary* find(std::string term);

	vocabulary *find(int id);

	void add(std::string term, int termID);

	int update(std::string voc);

	void update(std::string term, long loc, int len);

	void display();
	
};

#endif