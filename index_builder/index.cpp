#include <iostream>
#include <string>
#include <vector>
#include "index.hpp"
using namespace std;

void index::update(int termID, unsigned int docID, unsigned int position){
	vector<index::postingList>::iterator findterm = index::find(termID);
	if (findterm != ind.end()) {
		(findterm->postings).push_back(docID);
		(findterm->pos).push_back(position);
		// cout << findterm->term << endl;
		// for(vector<unsigned int>::iterator it2 = (findterm->postings).begin(); it2 != (findterm->postings).end(); ++it2){
		// 	cout << *it2 << ' ';
		// }
		// cout << endl;
	}
	else {
		postingList *posting = new postingList;
		posting->termID = termID;
		(posting->postings).push_back(docID);
		(posting->pos).push_back(position);
		ind.push_back(*posting);
	}
}

vector<index::postingList>::iterator index::find(int termID){
	vector<postingList>::iterator it;
	for(it = ind.begin(); it != ind.end(); ++ it){
		if(it->termID == termID){
			return it;
		}
	}
	return it;
}

void index::display(){
	for(vector<index::postingList>::iterator it = ind.begin(); it != ind.end(); ++ it){
		cout << it->termID << ' ';
		for(vector<unsigned int>::iterator intit = (it->postings).begin(); intit != (it->postings).end(); ++intit){
			cout<< *intit << " ";
		}
		cout << endl;
	}
}