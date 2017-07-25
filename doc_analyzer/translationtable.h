#ifndef TRANSLATIONTABLE_H
#define TRANSLATIONTABLE_H

#include <vector>
#include <unordered_map>

#include "translate.h"

class TranslationTable {
public:
    int apply(int docID, int version, int position);
    void insert(std::vector<Translation> trans, int docID);
    //If a document gets reindexed, throw away its translation list
    void erase(int docID);
    
private:
    //TEMPORARY DATA STRUCTURES
    //Holds a list of translations associated with a document
    //<docid, translations>
    std::unordered_map<int, std::vector<Translation>> transtable;
    //Holds a list of indexes that correspond to versions in a translation list
    //<docid, versions>
    //The index of versions denotes the version, the value denotes the position to begin applying from
    std::unordered_map<int, std::vector<int>> verstable;
};

#endif