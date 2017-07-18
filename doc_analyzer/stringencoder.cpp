#include "stringencoder.h"

#include <map>
#include <iostream>
#include <string.h>
#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;

StringEncoder::StringEncoder() : nextcode(0) {}

vector<int> StringEncoder::encodeFile(ifstream& file) {
    vector<int> encodedlist;
    if(!file) {
        cout << "Error: Could not open file" << endl;
        exit(EXIT_FAILURE);
    }
    
    string line;
    while(getline(file, line)) {
        //Make the line of text lowercase
        transform(line.begin(), line.end(), line.begin(), ::tolower);
        
        //Parse out any non-alphanumeric characters
        //Leave ' and -
        char* cstr = new char [line.length()+1];
        strcpy(cstr, line.c_str());

        char* p = strtok(cstr," ()`~!@#$%^&*+=|\\{}[]:;\"<>,.?/");
        while(p != 0)
        {
            string word(p);
            //Add the token/word into the dictionary if it's not in there yet,
            //giving it a unique identifier as indicated by nextcode
            if(dictionary.find(word) == dictionary.end()) {
                dictionary[word] = nextcode;
                lookup.push_back(word);
                nextcode++;
            }
            
            //Add the integer representation of the word into the list of ints
            encodedlist.push_back(dictionary[word]);
            
            p = strtok(NULL," ()`~!@#$%^&*+=|\\{}[]:;\"<>,.?/");
        }

        delete[] cstr;
    }
    
    return encodedlist;
}

//Decodes a stream of ints into a list of words
//Unknown ints are replaced with ??
vector<string> StringEncoder::decodeStream(vector<int>& stream) {
    vector<string> decodedlist;
    for(int i : stream) {
        if(i < lookup.size())
            decodedlist.push_back(lookup[i]);
        else
            decodedlist.push_back("??");
    }
    
    return decodedlist;
}