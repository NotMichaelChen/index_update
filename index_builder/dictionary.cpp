#include <string>
#include <vector>
#include <iostream>
#include "dictionary.hpp"
using namespace std;

Dictionary::vocabulary* Dictionary::find(string term){
	for(vector<Dictionary::vocabulary>::iterator it = dict.begin(); it != dict.end(); ++it){
		if(it->term == term){
			Dictionary::vocabulary* voc = new Dictionary::vocabulary;
			voc->term = term;
			voc->termID = it->termID;
			voc->location = it->location;
			voc->length = it->length;
			return voc;
		}
	}
	return nullptr;
}

Dictionary::vocabulary* Dictionary::find(int id){ //return a pointer to a vocabulary struct
	for(vector<Dictionary::vocabulary>::iterator it = Dictionary::dict.begin(); it != Dictionary::dict.end(); ++it){
		if(it->termID == id){
			Dictionary::vocabulary* voc = new Dictionary::vocabulary;
			voc->term = it->term;
			voc->termID = id;
			voc->location = it->location;
			voc->length = it->length;
			return voc;
		}
	}
	return nullptr;
}

void Dictionary::add(string term, int termID){
	Dictionary::dict.push_back({term, termID, 0, 0});
}

int Dictionary::update(string voc){
	if(!find(voc)){
		Dictionary::termID ++;
		add(voc, Dictionary::termID);
		return termID;
	}
}

void Dictionary::update(string term, long loc, int len){
	Dictionary::vocabulary* dicvoc = Dictionary::find(term);
	dicvoc->location = loc;
	dicvoc->length = len;
}

void Dictionary::display(){
	for(vector<Dictionary::vocabulary>::iterator it = dict.begin(); it != dict.end(); ++it){
		cout << it->term << ' ' << it->termID << endl;
	}
}