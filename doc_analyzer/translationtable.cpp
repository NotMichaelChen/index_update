#include "translationtable.h"

#include <vector>
#include <unordered_map>

#include "Matcher/translate.h"

using namespace std;

int TranslationTable::apply(int docID, int fragID, int position) {
    auto transptr = transtable.find(docID);
    //Probably should throw an error here, not having a transtable for a docID seems bad
    if(transptr == transtable.end())
        return -1;
    int finalposition = position;
    
    vector<Translation> translist = transptr->second;
    
    for(int i = fragID; i < translist.size(); ++i) {
        finalposition = applyTranslation(finalposition, translist[i]);
        //return if position becomes invalidated
        //cannot keep running; position might accidentally become revalidated
        if(finalposition < 0)
            return finalposition;
    }
    
    return finalposition;
}

void TranslationTable::insert(vector<Translation> trans, int docID) {
    auto existingtrans = transtable.find(docID);
    
    if(existingtrans != transtable.end()) {
        existingtrans->second.insert(existingtrans->second.end(), trans.begin(), trans.end());
    }
    else {
        transtable[docID] = trans;
    }
}

void TranslationTable::erase(int docID) {
    transtable.erase(docID);
}