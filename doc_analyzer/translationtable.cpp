#include "translationtable.h"

#include <vector>
#include <unordered_map>

#include "translate.h"

using namespace std;

int TranslationTable::apply(int docID, int version, int position) {
    vector<Translation> trans = transtable[docID];
    int finalposition = position;
    
    //Assumes version starts from 0
    int i = verstable[docID][version];
    while(i < trans.size()) {
        finalposition = applyTranslation(finalposition, trans[i]);
        //return if position becomes invalidated
        //cannot keep running; position might accidentally become revalidated
        if(finalposition < 0)
            return finalposition;
        
       ++i;
    }
    
    return finalposition
}

void TranslationTable::insert(vector<Translation> trans, int docID) {
    int nextindex = 0;
    
    auto existingtrans = transtable.find(docID);
    
    if(existingtrans != transtable.end()) {
        nextindex = *existingtrans->second.size();
        existingtrans->second.insert(existingtrans.end(), trans.begin(), trans.end());
    }
    else {
        transtable[docID] = trans;
    }
    
    auto existingvers = verstable.find(docID);
    
    if(existingvers != verstable.end()) {
        existingvers->second.push_back(nextindex);
    }
    else {
        vector<int> newverslist;
        newverslist.push_back(nextindex);
        existingvers[docID] = newverslist;
    }
}

void TranslationTable::erase(int docID) {
    transtable.erase(docID);
    verstable.erase(docID);
}