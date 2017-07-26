#ifndef TRANSLATIONTABLE_H
#define TRANSLATIONTABLE_H

#include <vector>
#include <unordered_map>

#include "translate.h"

class TranslationTable {
public:
    int apply(int docID, int fragID, int position);
    void insert(std::vector<Translation> trans, int docID);
    //If a document gets reindexed, throw away its translation list
    void erase(int docID);
    
private:
    //TEMPORARY DATA STRUCTURES
    //Holds a list of translations associated with a document
    //<docid, translations>
    //transtable index indicates where fragID should begin applying
    std::unordered_map<int, std::vector<Translation>> transtable;
};

#endif