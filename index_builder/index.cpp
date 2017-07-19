#include <iostream>
#include <string>
#include <vector>
using namespace std;
#include "index.hpp"


void Index::update(int termID, unsigned int docID, unsigned int position){
	vector<Index::postingList>::iterator findterm = Index::find(termID);
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

vector<Index::postingList>::iterator Index::find(int termID){
	vector<postingList>::iterator it;
	for(it = ind.begin(); it != ind.end(); ++ it){
		if(it->termID == termID){
			return it;
		}
	}
	return it;
}

void Index::display(){
	for(vector<Index::postingList>::iterator it = ind.begin(); it != ind.end(); ++ it){
		cout << it->termID << ' ';
		for(vector<unsigned int>::iterator intit = (it->postings).begin(); intit != (it->postings).end(); ++intit){
			cout<< *intit << " ";
		}
		cout << endl;
	}
}