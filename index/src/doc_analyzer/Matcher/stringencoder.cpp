#include "stringencoder.h"

#include <sstream>
#include <algorithm>

#include "util.hpp"

using namespace std;

namespace Matcher {
    //Returns whether punctuation was stripped
    bool stripPunctuation(string& s) {
        auto from = find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(ispunct)));
        auto to = find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(ispunct))).base();
        //Skip if word is just punctuation
        if(to == s.begin())
            return false;
        
        s = string(from, to);
        return true;
    }

    StringEncoder::StringEncoder(string oldfile, string newfile) : nextcode(0) {
        transform(oldfile.begin(), oldfile.end(), oldfile.begin(), ::tolower);
        transform(newfile.begin(), newfile.end(), newfile.begin(), ::tolower);

        vector<string> oldtokens = Utility::splitString(oldfile, " \n\t\r\f");
        vector<string> newtokens = Utility::splitString(newfile, " \n\t\r\f");

        for(string& token : oldtokens) {
            //Strip punctuation
            if(!stripPunctuation(token))
                continue;

            auto results = dictionary.insert(make_pair(token, nextcode));
            //If inserted
            if(results.second) {
                lookup.push_back(token);
                ++nextcode;
            }
            //For clarity
            auto dictiter = results.first;
            oldencoded.push_back(dictiter->second);
            oldexclusive.insert(token);
        }
        
        for(string& token : newtokens) {
            //Strip punctuation
            if(!stripPunctuation(token))
                continue;

            auto results = dictionary.insert(make_pair(token, nextcode));
            //If inserted
            if(results.second) {
                newcount[token] = 1;
                lookup.push_back(token);
                ++nextcode;
            } else {
                newcount[token] += 1;
            }
            //For clarity
            auto dictiter = results.first;
            newencoded.push_back(dictiter->second);
            newexclusive.insert(token);
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

        //Iterate through the smaller set and see if any of its elements are in the other set
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
