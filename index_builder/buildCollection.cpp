#include <iostream>
#include <fstream>
#include <boost/tokenizer.hpp>
#include <string>
#include <vector>
#include "dirent.h"
#include "index.hpp"
#include "dictionary.hpp"
using namespace std;
using namespace boost;

vector<std::string> tokenize(string text) {
	std::string s = text;
	tokenizer<> tok(s);
	vector<std::string> vocabulary;
	for (tokenizer<>::iterator beg = tok.begin(); beg != tok.end(); ++beg) {
		vocabulary.push_back(*beg);
	}
	return vocabulary;
}

string readFile(string dir) {
	ifstream myFile;
	myFile.open(dir.c_str());
	string inputStream;
	string result = "";
	while(getline(myFile, inputStream)){
		result += inputStream;
	}
	myFile.close();
	return result;
}

int main(){
	unsigned int docId = 0;
	Dictionary dictionary;
	Index index;
	DIR *dir;
	struct dirent *ent;

	if ((dir = opendir("./test_html")) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {
			string dir = ent->d_name;
			string wordStream;
			if (dir != "." && dir != "..") {
				docId ++;
				//cout << dir << endl;
				wordStream = readFile("./test_html/" + dir);
				vector<string> voc = tokenize(wordStream);
				unsigned int pos = 0;

				for(vector<string>::iterator it = voc.begin(); it != voc.end(); it ++){
					int termId = dictionary.update(*it);
					index.update(termId, docId, pos);
					pos ++;
				}
				
				//buildPList(voc, docId, index);
				//buildPageTable(fileDir, docId, pageTable);
			}
		}
		//dictionary.display();
		index.display();
		/*
		closedir(dir);
		displayPList(postingList);
		writetoDisk(postingList, dic);
		displayDic(dic);
		*/
	}
	else {
		/* could not open directory */
		perror("Cannot open directory.");
		return EXIT_FAILURE;
	}


	return 0;
}