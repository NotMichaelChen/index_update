#include "query_primitive.hpp"

#include <dirent.h>

#include "../global_parameters.hpp"


query_primitive::query_primitive(unsigned int termID, std::map<unsigned int, std::vector<nPosting>>& index, ExtendedLexicon& exlex) {
    lists.emplace_back(termID, index);

    for(auto iter = exlex.getNonPositionalBegin(termID); iter != exlex.getNonPositionalEnd(termID); iter++) {
        lists.emplace_back(termID, iter);
    }

    curdocIDs.resize(lists.size());
    docID = 0;
}

unsigned int query_primitive::nextGEQ(unsigned int x) {
    //TODO: Handle nextGEQ failures
    unsigned int min = docID;

    for(size_t i = 0; i < lists.size(); ++i) {
        unsigned int next = lists[i].nextGEQ(x);
        curdocIDs[i] = next;
        if(next < min)
            min = next;
    }

    docID = min;
    return min;
}

unsigned int query_primitive::getFreq() {
}

