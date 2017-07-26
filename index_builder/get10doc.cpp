#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include <cmath>
#include <algorithm>
#include <sstream>
#define DATA "./test"
#define NO_DOC 10
#define INDEX "/home/qi/forwardIndex/compressedIndex"
#define INFO "/home/qi/forwardIndex/docInfo"
using namespace std;

int main(){
	ifstream index;
	ifstream info;
	index.open(INDEX);
	info.open(INFO);

	ofstream newindex("./compressedIndex");
	ofstream newinfo("./docInfo");

	string line;
	string value;
	vector<string> vec;
	char c;
	int p;
	int num;

	for(int i = 0; i < NO_DOC; i ++){	
		vec.clear();
		getline(info, line);//read docInfo
		newinfo << line;
		stringstream lineStream(line);
		unsigned int pos = 0;

		while(lineStream >> value){
			vec.push_back(value);
		}

		index.seekg(stoi(vec[2]));
		while(index.tellg() != (stoi(vec[2]) + stoi(vec[3]))){
			//for every document, do
			index.get(c);
			newindex << c;

		}
	}
	index.close();
	info.close();
	return 0;
}