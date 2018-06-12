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

            auto results = dictionary.insert(make_pair(word, nextcode));
            //If inserted
            if(results.second) {
                lookup.push_back(word);
                ++nextcode;
            }
            //For clarity
            auto iter = results.first;
            oldencoded.push_back(iter->second);
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

            auto results = dictionary.insert(make_pair(word, nextcode));
            //If inserted
            if(results.second) {
                newcount[word] = 1;
                lookup.push_back(word);
                ++nextcode;
            } else {
                newcount[word] += 1;
            }
            //For clarity
            auto iter = results.first;
            newencoded.push_back(iter->second);
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

        //Iteratre through the smaller set and see if any of its elements are in the other set
        auto iter = iterateset->begin();
        while(iter != iterateset->end()) {

            auto compareiter = compareset->find(*iter);
            if(compareiter != compareset->end()) {
                compareset->erase(*iter);

                //Store the next position since this iterator will be invalidated
                auto newiter = iter;
                newiter++;

                iterateset->erase(iter);
                
                iter = newiter;
            }
            else {
                iter++;
            }
        }
    }
    
    //Decodes a stream of ints into a list of words
    //Unknown ints are replaced with ??
    vector<string> StringEncoder::decodeStream(vector<unsigned int>& stream) {
        vector<string> decodedlist;
        for(unsigned int i : stream) {
            if(i < lookup.size())
                decodedlist.push_back(lookup[i]);
            else
                decodedlist.push_back("??");
        }
        
        return decodedlist;
    }
    
    //Decode an individual int
    //An unknown int returns ??
    string StringEncoder::decodeNum(unsigned int num) {
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
