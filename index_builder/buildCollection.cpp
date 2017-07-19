#include <iostream>
#include <fstream>
#include <boost/tokenizer.hpp>
#include <string>
#include <vector>
#include <sstream>
#include "dirent.h"
#include "index.hpp"
#include "dictionary.hpp"
#define TEST_DATA "./test_data"
using namespace std;
using namespace boost;



int main(){
	unsigned int docId = 0;
	Dictionary dictionary;
	Index index;

	ifstream docs;
	ifstream info;
	docs.open(TEST_DATA + compressedIndex, ifstream::binary);
	info.open(TEST_DATA + docInfo);

	string line;
	int fileNum = 0;
	while(getline(info, line) && fileNum < 10){
		stringstream lineStream(line);
        string fileName;
        int docID;
        int offset;
        int size;
        int docLength;
        lineStream >> fileName >> docID >> offset >> size >> docLength;
        
        fileNUM ++;
	}


	

	return 0;
}
/*vector<std::string> tokenize(string text) {
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
DIR *dir;
	struct dirent *ent;

	if ((dir = opendir(TEST_DATA)) != NULL) {
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
			}
		}
		//dictionary.display();
		index.display();
		/*
		closedir(dir);
		displayPList(postingList);
		writetoDisk(postingList, dic);
		displayDic(dic);
		
	}
	else {
		/* could not open directory 
		perror("Cannot open directory.");
		return EXIT_FAILURE;
	}

*/