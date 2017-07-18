#include <string>
#include <vector>
#include "dictionary.hpp"
using namespace std;

vocabulary* dictionary::find(string term){
	for(vector<dictionary::vocabulary>::iterator it = dict.begin(); it != dict.end(); ++it){
		if(it->term == term){
			vocabulary* voc = new vocabulary;
			voc->term = term;
			voc->termID = it->termID;
			voc->location = it->location;
			voc->length = it->length;
			return voc;
		}
	}
	return nullptr;
}

vocabulary* dictionary::find(int id){ //return a pointer to a vocabulary struct
	for(vector<dictionary::vocabulary>::iterator it = dict.begin(); it != dict.end(); ++it){
		if(it->termID == id){
			vocabulary* voc = new vocabulary;
			voc->term = it->term;
			voc->termID = id;
			voc->location = it->location;
			voc->length = it->length;
			return voc;
		}
	}
	return nullptr;
}

void dictionary::add(string term, int termID){
	dict.push_back({term, termID, 0, 0});
}

int dictionary::update(string voc){
	if(!find(voc)){
		termID ++;
		add(voc, termID);
		return termID;
	}
}

void dictionary::update(string term, long loc, int len){
	vocabulary* dicvoc = find(term);
	dicvoc->location = loc;
	dicvoc->length = len;
}

void dictionary::display(){
	for(vector<dictionary::vocabulary>::iterator it = dict.begin(); it != dict.end(); ++it){
		cout << it->term << ' ' << it->termID << endl;
	}
}