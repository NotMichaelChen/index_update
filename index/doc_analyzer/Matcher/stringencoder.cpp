#include "stringencoder.h"

#include <map>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;

namespace Matcher {
    StringEncoder::StringEncoder(string& oldfile, string& newfile) : nextcode(0) {
        //All terms found in the old map
        unordered_map<string, int> oldmap;
        stringstream oldstream(oldfile);
        
        string word;
        while(oldstream >> word) {
            transform(word.begin(), word.end(), word.begin(), ::tolower);
            
            if(oldmap.find(word) == oldmap.end()) {
                oldmap[word] = nextcode;
                dictionary[word] = nextcode;
                lookup.push_back(word);
                ++nextcode;
            }
            
            oldencoded.push_back(dictionary[word]);
        }
        
        //Terms found only in the new file
        unordered_map<string, int> newmap;
        stringstream newstream(newfile);
        while(newstream >> word) {
            transform(word.begin(), word.end(), word.begin(), ::tolower);
            
            if(dictionary.find(word) == dictionary.end()) {
                newmap[word] = nextcode;
                dictionary[word] = nextcode;
                lookup.push_back(word);
                ++nextcode;
            }
            else {
                oldmap.erase(word);
            }
            
            newencoded.push_back(dictionary[word]);
        }
        
        oldexclusive.reserve(oldmap.size());
        newexclusive.reserve(newmap.size());
        
        for(auto kv : oldmap) {
            oldexclusive.insert(kv.first);
        }
        for(auto kv : newmap) {
            newexclusive.insert(kv.first);
        }
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
    
    bool StringEncoder::inOld(string& word) {
		return oldexclusive.find(word) != oldexclusive.end();
	}
    
    bool StringEncoder::inNew(string& word) {
		return newexclusive.find(word) != newexclusive.end();
	}
    
    vector<int>::const_iterator StringEncoder::getOldIter() {
        return oldencoded.cbegin();
    }
    
    vector<int>::const_iterator StringEncoder::getOldEnd() {
        return oldencoded.cend();
    }
    
    vector<int>::const_iterator StringEncoder::getNewIter() {
        return newencoded.cbegin();
    }
    
    vector<int>::const_iterator StringEncoder::getNewEnd() {
        return newencoded.cend();
    }
    
    int StringEncoder::getOldSize() {
        return oldencoded.size();
    }
    
    int StringEncoder::getNewSize() {
        return newencoded.size();
    }
}
