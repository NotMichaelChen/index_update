#include "stringencoder.h"

#include <map>
#include <iostream>
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
    
    //Assume given file is already parsed
    
    string word;
    while(file >> word) {
        //Make the word lowercase
        transform(word.begin(), word.end(), word.begin(), ::tolower);
        
        //Add the token/word into the dictionary if it's not in there yet,
        //giving it a unique identifier as indicated by nextcode
        if(dictionary.find(word) == dictionary.end()) {
            dictionary[word] = nextcode;
            lookup.push_back(word);
            nextcode++;
        }
        
        //Add the integer representation of the word into the list of ints
        encodedlist.push_back(dictionary[word]);
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

//Decode an individual int
//An unknown int returns ??
string StringEncoder::decodeNum(int num) {
    if(num < lookup.size())
        return lookup[num];
    else return "??";
}