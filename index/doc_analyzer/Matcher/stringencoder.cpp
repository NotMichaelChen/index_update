#include "stringencoder.h"

#include <sstream>
#include <algorithm>

using namespace std;

namespace Matcher {
    StringEncoder::StringEncoder(string& oldfile, string& newfile) : nextcode(0) {
        string word;
        stringstream oldstream(oldfile);
        while(oldstream >> word) {
            transform(word.begin(), word.end(), word.begin(), ::tolower);
            //Strip punctuation
            auto from = find_if(word.begin(), word.end(), not1(ptr_fun<int, int>(ispunct)));
            auto to = find_if(word.rbegin(), word.rend(), not1(ptr_fun<int, int>(ispunct))).base();
            //Skip if word is just punctuation
            if(to == word.begin())
                continue;
            
            word = string(from, to);
            
            if(dictionary.find(word) == dictionary.end()) {
                dictionary[word] = nextcode;
                lookup.push_back(word);
                ++nextcode;
            }
            
            oldencoded.push_back(dictionary[word]);
            oldexclusive.insert(word);
        }
        
        stringstream newstream(newfile);
        while(newstream >> word) {
            transform(word.begin(), word.end(), word.begin(), ::tolower);
            //Strip punctuation
            auto from = find_if(word.begin(), word.end(), not1(ptr_fun<int, int>(ispunct)));
            auto to = find_if(word.rbegin(), word.rend(), not1(ptr_fun<int, int>(ispunct))).base();
            //Skip if word is just punctuation
            if(to == word.begin())
                continue;
            
            word = string(from, to);
            
            if(dictionary.find(word) == dictionary.end()) {
                dictionary[word] = nextcode;
                newcount[word] = 1;
                lookup.push_back(word);
                ++nextcode;
            }
            else {
                newcount[word] += 1;
            }
            
            newencoded.push_back(dictionary[word]);
            newexclusive.insert(word);
        }

        //Create a pointer to the smaller set so we don't have to iterate through as many elements
        unordered_set<string>* iterateset;
        unordered_set<string>* compareset;
        if(oldexclusive.size() > newexclusive.size()) {
            iterateset = &newexclusive;
            compareset = &oldexclusive;
        }
        else {
            iterateset = &newexclusive;
            compareset = &oldexclusive;
        }

        for(string key : *iterateset) {
            if(compareset->find(key) != compareset->end()) {
                compareset->erase(key);
            }
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

    int StringEncoder::getNewCount(string word) {
        return newcount[word];
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
