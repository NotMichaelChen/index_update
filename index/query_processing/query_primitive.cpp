#include "query_primitive.hpp"

#include <dirent.h>
#include <limits>

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
    unsigned int min = std::numeric_limits<unsigned int>::max();

    for(size_t i = 0; i < lists.size(); ++i) {
        bool failure = false;
        unsigned int next = lists[i].nextGEQ(x, failure);
        if(failure)
            curdocIDs[i] = std::numeric_limits<unsigned int>::max();
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
