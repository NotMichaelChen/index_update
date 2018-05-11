#include "query_primitive.hpp"

#include <dirent.h>

#include "global_parameters.hpp"

query_primitive::query_primitive(unsigned int termID, GlobalType::NonPosIndex& index, ExtendedLexicon& exlex) {
    lists.emplace_back(termID, index);

    for(auto iter = exlex.getNonPositionalBegin(termID); iter != exlex.getNonPositionalEnd(termID); iter++) {
        lists.emplace_back(termID, iter);
    }

    curdocIDs.resize(lists.size());
    docID = 0;
}

unsigned int query_primitive::nextGEQ(unsigned int x) {
    //Finds the QP with the smallest docID greater than x
    unsigned int min = GlobalConst::UIntMax;

    //Loop through every low level QP and call nextGEQ on it
    for(size_t i = 0; i < lists.size(); ++i) {
        bool failure = false;
        unsigned int next = lists[i].nextGEQ(x, failure);
        if(failure)
            curdocIDs[i] = GlobalConst::UIntMax;
        else {
            curdocIDs[i] = next;
            if(next <= min)
                min = next;
        }
    }
    docID = min;
    return min;
}

unsigned int query_primitive::getFreq() {
    //Make larger than any possible index num, specific number doesn't matter
    int minindexnum = lists.size();
    unsigned int freq = 0;

    //Loop through all the QP, find the pointer to the smallest index number, and get its frequency
    for(size_t i = 0; i < lists.size(); ++i) {
        if(curdocIDs[i] == docID) {
            int newindexnum = lists[i].getIndexNumber();

            if(newindexnum < minindexnum) {
                minindexnum = newindexnum;
                freq = lists[i].getFreq();
            }
        }
    }

    return freq;
}
